#include "sc_tui.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <string>
#include <vector>

#include "ftxui/component/component.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/mouse.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/terminal.hpp"

#include "sc_editcore.hpp"

using namespace ftxui;

namespace scedit {
namespace {

// --- geometry ---------------------------------------------------------------
// Explicit, not flexed: the self-test renders into a fixed screen and compares
// the frame, so every row must be where the code says it is.
constexpr int kGutter = 10;    //!< severity + line number + separator, in cells
constexpr int kDocRows = 4;    //!< the documentation bar (see renderDocBar)
constexpr int kPaneRows = 4;   //!< entry rows of the error pane (see renderPane)
constexpr int kFeedRows = 5;   //!< lines of the live feed (see renderFeed)
//! title + separator + [text] + separator + doc bar + separator + status
constexpr int kChrome = 1 + 1 + 1 + kDocRows + 1 + 1;
//! separator + header + the entry rows, added to kChrome while the pane is open
constexpr int kPaneChrome = 1 + 1 + kPaneRows;
//! idem for the feed pane
constexpr int kFeedChrome = 1 + 1 + kFeedRows;

//! HOW OFTEN the two clocked things happen. Both bounds are in the header's
//! contract and in README S Live mode, because "it polls" with no number is not
//! a statement anybody can check.
constexpr long kDrainMs = 250;        //!< drain the socket (reading what was pushed)
constexpr long kFileCheckMs = 1000;   //!< re-read the played file: never faster than 1 Hz
constexpr long kPlayWindowMs = 300000;//!< and never longer than five minutes after a play

//! One byte of the buffer -> one display cell. See sc_tui.hpp for the rules.
struct Glyph {
	std::string s;
	bool lookalike = false;   //!< looks like a space, is not one to the engine
	bool control = false;     //!< invisible byte shown as a marker
};

Glyph glyphFor(unsigned char b)
{
	Glyph g;
	if (b == 0xA0) {
		g.s = "\xC2\xB7";        // U+00B7 MIDDLE DOT
		g.lookalike = true;
		return g;
	}
	if (b == '\t') { g.s = "\xC2\xBB"; g.control = true; return g; }   // U+00BB
	if (b == '\r') { g.s = "\xE2\x86\xB5"; g.control = true; return g; }   // U+21B5
	if (b < 0x20 || b == 0x7F) { g.s = "?"; g.control = true; return g; }
	if (b < 0x80) { g.s = std::string(1, (char)b); return g; }
	// ISO-8859-1: the byte IS the code point. Display only; the buffer keeps
	// the byte, and a save writes the byte back.
	g.s.push_back((char)(0xC0 | (b >> 6)));
	g.s.push_back((char)(0x80 | (b & 0x3F)));
	return g;
}

//! Truncate to `cells` code points (every glyph this file draws is one cell).
std::string truncate(const std::string &s, int cells)
{
	if (cells <= 0)
		return std::string();
	int n = 0;
	std::size_t i = 0;
	while (i < s.size() && n < cells) {
		const unsigned char c = (unsigned char)s[i];
		std::size_t len = 1;
		if ((c & 0xE0) == 0xC0) len = 2;
		else if ((c & 0xF0) == 0xE0) len = 3;
		else if ((c & 0xF8) == 0xF0) len = 4;
		if (i + len > s.size()) len = 1;
		i += len;
		++n;
	}
	return s.substr(0, i);
}

std::string severityMark(const std::string &sev)
{
	if (sev == "error") return "E";
	if (sev == "warning") return "W";
	if (sev == "info") return "i";
	return " ";
}

Color severityColor(const std::string &sev)
{
	if (sev == "error") return Color::Red;
	if (sev == "warning") return Color::Yellow;
	if (sev == "info") return Color::Cyan;
	return Color::Default;
}

std::string opennessLabel(Openness o)
{
	switch (o) {
	case Openness::Exhaustive: return "all of them";
	case Openness::Open: return "known ones \xe2\x80\x94 there are more";
	case Openness::Unstated: return "";
	}
	return "";
}

//! What the feed pane draws, as data. The LINES are borrowed, not copied: the
//! pointer is set immediately before a render and is a `TcpClient`'s own deque
//! (or, in the self-test, a local one) -- non-owning, valid for the render call
//! only (I5). Everything else is a snapshot of the link's state, so the
//! renderer never asks a socket anything.
struct FeedView {
	bool enabled = false;                    //!< --tcp was given
	std::string endpoint = "127.0.0.1:7805";
	LinkState state = LinkState::Offline;
	std::size_t dropped = 0;
	std::size_t sent = 0;
	std::size_t bytes = 0;
	bool playing = false;                    //!< a play is in flight (the poll window)
	const std::deque<FeedLine> *lines = nullptr;
	//! How many lines back from the newest the pane's bottom sits. 0 = live.
	std::size_t back = 0;
	std::size_t count() const { return lines ? lines->size() : 0; }
};

//! What the editor is showing, over and above what the core knows.
struct View {
	std::size_t top = 0;        //!< first buffer line drawn
	std::size_t hscroll = 0;    //!< first byte column drawn
	//! Whether the view must chase the caret. A key press turns it on; the
	//! wheel turns it off, because scrolling away from the caret is the whole
	//! point of scrolling and a view that snaps straight back has not scrolled.
	bool follow = true;
	//! Is the error pane open? The pane costs kPaneChrome rows of text, and
	//! most files are clean, so it is off until asked for -- the count is on the
	//! status line at all times, which is what makes it findable (README S
	//! The error pane; a scedit UX call, veto open).
	bool pane = false;
	//! The pane's row, as of the last warp. Meaningless unless the caret still
	//! stands where that row begins (paneCursor); npos-like values are simply
	//! out of range and are treated as "not on a row".
	std::size_t sel = (std::size_t)-1;
	std::string status;
	//! Is the live feed pane open? Same reasoning as `pane`: it costs rows, and
	//! the status line carries its state at all times so it is findable.
	bool feedPane = false;
	FeedView feed;
	int width = 100;
	int height = 30;
	Box textBox;                //!< filled by the renderer, read by the mouse
	Box paneBox;                //!< idem, for the pane's rows
	Box feedBox;                //!< idem, for the feed's rows (the wheel scrolls it)
};

int textRows(const View &v)
{
	return std::max(1, v.height - kChrome - (v.pane ? kPaneChrome : 0)
	                   - (v.feedPane ? kFeedChrome : 0));
}
int textCols(const View &v) { return std::max(1, v.width - kGutter); }

// --- the error pane ---------------------------------------------------------
// The pane keeps ONE piece of state, `View::sel`: the row the last warp landed
// on. It is trusted only while the caret still stands exactly where that row
// begins -- the moment the author moves or types, it is recomputed from the
// caret. So an edit can never leave the pane pointing at a row that no longer
// exists, and two rows that begin on the same byte are still told apart while
// stepping (which a purely derived cursor cannot do).
//
// F3/F4 walk the LIST, not the file: on one line the engine's row comes before
// scedit's although its `#!` sits further right, and a stepper that followed
// byte order would skip that row forever.

//! The row the caret is standing on, or npos.
std::size_t paneCursor(const EditCore &core, const View &v)
{
	const std::vector<ErrorEntry> &h = core.errorHistory();
	if (v.sel >= h.size())
		return std::string::npos;
	const ErrorEntry &e = h[v.sel];
	return e.line == core.cursor().line + 1
	       && (e.span.empty() ? 0 : e.span.begin) == core.cursor().col
	       ? v.sel : std::string::npos;
}

//! The first row at or after the caret's line, else the size (past the end).
std::size_t paneFocus(const EditCore &core, const View &v)
{
	const std::vector<ErrorEntry> &h = core.errorHistory();
	const std::size_t cur = paneCursor(core, v);
	if (cur != std::string::npos)
		return cur;
	const std::size_t here = core.cursor().line + 1;
	std::size_t i = 0;
	while (i < h.size() && h[i].line < here)
		++i;
	return i;
}

//! Which row sits at the top of the pane: derived from the focus, so the same
//! caret always scrolls the pane the same way. The focus is kept near the
//! middle when it can be.
std::size_t paneTop(const EditCore &core, const View &v)
{
	const std::size_t n = core.errorHistory().size();
	const std::size_t rows = (std::size_t)kPaneRows;
	if (n <= rows)
		return 0;
	const std::size_t focus = std::min(paneFocus(core, v), n - 1);
	std::size_t top = focus < rows / 2 ? 0 : focus - rows / 2;
	if (top + rows > n)
		top = n - rows;
	return top;
}

//! Put the caret on row `i` and remember it as the pane's row.
void warpToRow(EditCore &core, View &v, std::size_t i)
{
	const std::vector<ErrorEntry> &h = core.errorHistory();
	if (i >= h.size())
		return;
	// By VALUE: warpTo must never be handed a reference into the list it may
	// rebuild.
	const ErrorEntry e = h[i];
	v.sel = i;
	core.warpTo(e);
}

//! Warp to the row after (delta > 0) or before (delta < 0) the one the caret is
//! on, wrapping. When the caret is not on a row, enter the list at the nearest
//! one in the direction of travel -- so F3 from the top of a file goes to the
//! pane's first row, which is what it says on the screen.
void warpStep(EditCore &core, View &v, int delta)
{
	const std::vector<ErrorEntry> &h = core.errorHistory();
	if (h.empty())
		return;
	const std::size_t n = h.size();
	const std::size_t cur = paneCursor(core, v);
	if (cur != std::string::npos) {
		warpToRow(core, v, delta > 0 ? (cur + 1) % n : (cur + n - 1) % n);
		return;
	}
	const std::size_t line = core.cursor().line + 1, col = core.cursor().col;
	auto beginOf = [](const ErrorEntry &e) { return e.span.empty() ? (std::size_t)0 : e.span.begin; };
	if (delta > 0) {
		for (std::size_t i = 0; i < n; ++i)
			if (h[i].line > line || (h[i].line == line && beginOf(h[i]) > col))
				{ warpToRow(core, v, i); return; }
		warpToRow(core, v, 0);       // wrap
		return;
	}
	for (std::size_t i = n; i-- > 0;)
		if (h[i].line < line || (h[i].line == line && beginOf(h[i]) < col))
			{ warpToRow(core, v, i); return; }
	warpToRow(core, v, n - 1);       // wrap
}

//! Keep the caret on screen. Called before rendering and after every move.
void scrollToCursor(const EditCore &core, View &v)
{
	const std::size_t rows = (std::size_t)textRows(v);
	if (core.cursor().line < v.top)
		v.top = core.cursor().line;
	else if (core.cursor().line >= v.top + rows)
		v.top = core.cursor().line - rows + 1;

	const std::size_t cols = (std::size_t)textCols(v);
	// The ghost is drawn at the caret, so keep a little of it visible too.
	const std::size_t want = core.cursor().col;
	if (want < v.hscroll)
		v.hscroll = want;
	else if (want >= v.hscroll + cols)
		v.hscroll = want - cols + 1;
}

// --- rendering --------------------------------------------------------------

//! One buffer line, as screen cells: gutter, then the bytes from `hscroll`,
//! with the ghost text inserted (greyed) at the caret.
Element renderLine(const EditCore &core, const View &v, std::size_t lineNo, bool isCursorLine)
{
	const std::string &raw = core.document().line(lineNo);
	const std::string sev = core.severityForLine(lineNo + 1);
	// What is marked is what the rules decided, read off their spans: the red
	// look-alike-space marker on `invisible-separator`'s bytes, an underline on
	// every other finding's bytes. No second reading of the bytes here.
	const std::vector<const Diagnostic *> diags = core.diagnosticsForLine(lineNo + 1);
	auto lookAt = [&diags](std::size_t i) {
		for (const auto *d : diags)
			if (d->id == "invisible-separator" && d->span.contains(i))
				return true;
		return false;
	};
	auto underAt = [&diags](std::size_t i) {
		for (const auto *d : diags)
			if (d->id != "invisible-separator" && !d->span.empty() && d->span.contains(i))
				return true;
		return false;
	};
	// The comment tail: bytes the engine never reads, drawn dim like every
	// other thing on this screen that is not executed (the ghost included).
	const std::size_t commentAt = core.commentBegin(lineNo);

	Elements gut;
	gut.push_back(text(severityMark(sev)) | color(severityColor(sev)) | bold);
	char num[16];
	std::snprintf(num, sizeof(num), " %5zu ", lineNo + 1);
	gut.push_back(text(num) | dim);
	gut.push_back(text("\xE2\x94\x82 "));   // U+2502 light vertical

	const std::string ghost = isCursorLine ? core.completion().ghost() : std::string();
	const std::size_t caret = core.cursor().col;

	// Build the cell run, then group consecutive cells of the same style.
	struct Cell { std::string s; bool look, under, ctrl, comment, ghosty, caretHere; };
	std::vector<Cell> cells;
	for (std::size_t i = 0; i <= raw.size(); ++i) {
		if (isCursorLine && i == caret) {
			for (std::size_t k = 0; k < ghost.size(); ++k) {
				const Glyph g = glyphFor((unsigned char)ghost[k]);
				cells.push_back(Cell{g.s, false, false, false, false, true, k == 0});
			}
			if (ghost.empty() && i < raw.size()) {
				const Glyph g = glyphFor((unsigned char)raw[i]);
				cells.push_back(Cell{g.s, lookAt(i), underAt(i), g.control, i >= commentAt, false, true});
				continue;
			}
			if (ghost.empty() && i == raw.size())
				cells.push_back(Cell{" ", false, false, false, false, false, true});
		}
		if (i < raw.size()) {
			const Glyph g = glyphFor((unsigned char)raw[i]);
			// The GLYPH is this layer's business; whether the byte is part of
			// a finding, or of a comment, is the tokenizer's/checker's answer.
			cells.push_back(Cell{g.s, lookAt(i), underAt(i), g.control, i >= commentAt, false, false});
		}
	}

	Elements runs;
	const std::size_t from = v.hscroll;
	const std::size_t to = std::min(cells.size(), from + (std::size_t)textCols(v));
	std::size_t i = from;
	while (i < to) {
		const Cell &c0 = cells[i];
		std::string run;
		std::size_t j = i;
		while (j < to && cells[j].look == c0.look && cells[j].under == c0.under
		       && cells[j].ctrl == c0.ctrl && cells[j].comment == c0.comment
		       && cells[j].ghosty == c0.ghosty && cells[j].caretHere == c0.caretHere) {
			run += cells[j].s;
			++j;
		}
		Element e = text(run);
		if (c0.ghosty)
			e = e | dim | color(Color::GrayDark);
		else if (c0.look)
			e = e | color(Color::Red) | bold;
		else if (c0.comment)
			e = e | dim;
		else if (c0.ctrl)
			e = e | dim;
		if (c0.under)
			e = e | underlined;
		// The caret is the terminal's standard foreground/background inversion
		// (SGR 7): visible on every palette, and what a reader expects in a tui.
		if (c0.caretHere)
			e = e | inverted;
		runs.push_back(e);
		i = j;
	}
	if (runs.empty())
		runs.push_back(text(""));

	Elements all = gut;
	all.insert(all.end(), runs.begin(), runs.end());
	return hbox(std::move(all));
}

Element renderDocBar(const EditCore &core, const View &v)
{
	const DocBar &d = core.docBar();
	const Completion &c = core.completion();
	const int w = v.width - 2;

	// Row 1: where the caret is, and what would complete there.
	std::string row1 = d.path.empty() ? std::string("\xe2\x80\x94") : d.path;
	if (!c.candidates.empty()) {
		row1 += "   [" + c.what + ": " + std::to_string(c.candidates.size());
		const std::string o = opennessLabel(c.openness);
		if (!o.empty())
			row1 += ", " + o;
		row1 += "]";
	}

	// Row 2: the sentence, or the honest blank.
	Element row2;
	if (d.documented) {
		std::string t = d.doc;
		if (!d.doc_of.empty())
			t += "   (documents: " + d.doc_of + ")";
		row2 = text(truncate(t, w));   // never paragraph(): wrapping would move every row below
	} else {
		row2 = text(truncate(kNoDoc, w)) | dim | color(Color::GrayLight);
	}

	// Row 3: the value domain, the values it names, the default.
	std::string row3;
	if (!d.domain.empty())
		row3 = "value: " + d.domain;
	if (!d.values.empty()) {
		row3 += row3.empty() ? "one of: " : "   one of: ";
		for (std::size_t i = 0; i < d.values.size(); ++i)
			row3 += (i ? ", " : "") + d.values[i];
	}
	if (!d.def.empty())
		row3 += (row3.empty() ? "" : "   ") + std::string("default: ") + d.def;
	if (!d.required.empty())
		row3 += "   required: " + d.required;

	// Row 4: the findings on this line, else the note, else the engine anchor.
	Element row4;
	const std::vector<const Diagnostic *> diags = core.diagnosticsForLine(core.cursor().line + 1);
	if (!diags.empty() || !d.annotation.empty()) {
		std::string t;
		for (const auto *dg : diags)
			t += (t.empty() ? "" : " | ") + dg->severity + ": " + dg->message + " [-W" + dg->id + "]";
		// What the ENGINE wrote on this line the last time it ran the script,
		// and whether scedit's reading agrees (MachineTail::relation).
		if (!d.annotation.empty())
			t += (t.empty() ? "" : " | ") + std::string("spacecrafter wrote #! ") + d.annotation;
		const bool agree = d.annotation.empty() || d.annotation.find("agrees with") != std::string::npos;
		row4 = text(truncate(t, w)) | color(!diags.empty() && agree ? severityColor(diags.front()->severity) : Color::Yellow);
	} else if (!d.note.empty()) {
		row4 = text(truncate(d.note, w)) | color(Color::Yellow);
	} else {
		row4 = text(truncate(d.source.empty() ? std::string() : "engine: " + d.source, w)) | dim;
	}

	return vbox({
		text(truncate(row1, w)) | bold,
		row2,
		text(truncate(row3, w)),
		row4,
	});
}

//! The error pane: every `#!` tail spacecrafter wrote and every finding scedit
//! makes, in line order, click-to-warp (scedit/INTENT.md S5 item 15(a-ii)).
//! A row's shape: `> E    12 | sc  unknown-command: message`, where the leading
//! `>` (and the inversion) mark EVERY entry on the caret's line -- "the caret's
//! entry", derived rather than remembered. An engine row is marked `!` and `#!`
//! because the engine states no severity of its own and scedit will not invent
//! one for it (ErrorEntry::severity).
Element renderPane(const EditCore &core, View &v)
{
	const std::vector<ErrorEntry> &h = core.errorHistory();
	std::size_t engine = 0;
	for (const ErrorEntry &e : h)
		if (e.source == EntrySource::Engine)
			++engine;

	std::string head;
	if (h.empty()) {
		head = "errors 0 \xE2\x80\x94 nothing to go to: no finding, no #! tail";
	} else {
		head = "errors " + std::to_string(h.size()) + " \xE2\x80\x94 "
		       + std::to_string(engine) + " spacecrafter, " + std::to_string(h.size() - engine) + " scedit";
	}
	head += "   \xC2\xB7 F3/F4 next/prev \xC2\xB7 F5 hide \xC2\xB7 click to warp";

	Elements rows;
	const std::size_t top = paneTop(core, v);
	const std::size_t here = core.cursor().line + 1;
	// `>` marks the row the caret STANDS on when there is one; when the author
	// has moved somewhere the pane did not send them, it marks every row of the
	// line they are on, which is still exactly "the caret's entry".
	const std::size_t cur = paneCursor(core, v);
	for (int r = 0; r < kPaneRows; ++r) {
		const std::size_t i = top + (std::size_t)r;
		if (i >= h.size()) {
			rows.push_back(text(""));
			continue;
		}
		const ErrorEntry &e = h[i];
		const bool engineRow = e.source == EntrySource::Engine;
		const bool onCaret = cur != std::string::npos ? i == cur : e.line == here;
		const std::string mark = engineRow ? std::string("!") : severityMark(e.severity);
		char pre[32];
		std::snprintf(pre, sizeof(pre), "%c%s %5zu \xE2\x94\x82 %s  ",
		              onCaret ? '>' : ' ', mark.c_str(), e.line, engineRow ? "#!" : "sc");
		std::string body = engineRow ? e.message : e.id + ": " + e.message;
		// One cell per code point in `pre`: 8 ASCII + the U+2502.
		Element row = hbox({text(pre), text(truncate(body, std::max(0, v.width - 14)))});
		row = row | color(engineRow ? Color::Yellow : severityColor(e.severity));
		if (onCaret)
			row = row | inverted;
		rows.push_back(row);
	}

	return vbox({
		text(truncate(head, v.width)) | dim,
		vbox(std::move(rows)) | reflect(v.paneBox),
	});
}

//! The live feed: what scedit sent (marked `>`, dim -- it is not the engine
//! speaking) and what the engine sent back, newest at the bottom.
//!
//! WHAT THE HEADER SAYS, and why each part is there: the endpoint and the link
//! state, because "nothing is happening" has two very different causes; the
//! line count and the DROPPED count, because a bounded buffer that discards in
//! silence is a buffer that lies; `+N` when the pane has been scrolled back, so
//! nobody reads an old line as the latest; and `playing` while the write-back
//! window is open, which is the only period in which anything here touches the
//! clock.
Element renderFeed(View &v)
{
	const FeedView &f = v.feed;
	const std::size_t n = f.count();

	std::string state;
	switch (f.state) {
	case LinkState::Connected: state = "connected"; break;
	case LinkState::Offline:   state = "not connected"; break;
	case LinkState::Failed:    state = "connection failed"; break;
	}
	std::string head = "live " + f.endpoint + " \xE2\x80\x94 " + state;
	if (f.state == LinkState::Connected)
		head += ", " + std::to_string(f.sent) + " sent, " + std::to_string(f.bytes) + " B in";
	if (f.playing)
		head += ", playing (watching the file)";
	head += "   \xC2\xB7 " + std::to_string(n) + " lines";
	if (f.dropped)
		head += ", " + std::to_string(f.dropped) + " dropped";
	if (f.back)
		head += ", +" + std::to_string(f.back) + " newer below";
	head += "   \xC2\xB7 F6 connect \xC2\xB7 F7 send line \xC2\xB7 F8 play \xC2\xB7 F11/F12 scroll \xC2\xB7 F9 hide";

	// The window: `back` counts lines from the newest, so back == 0 is live.
	const std::size_t rows = (std::size_t)kFeedRows;
	const std::size_t back = std::min(f.back, n > rows ? n - rows : (std::size_t)0);
	const std::size_t last = n - back;                     // one past the newest shown
	const std::size_t first = last > rows ? last - rows : 0;

	Elements out;
	for (std::size_t r = 0; r < rows; ++r) {
		const std::size_t i = first + r;
		if (i >= last || f.lines == nullptr) {
			out.push_back(text(""));
			continue;
		}
		const FeedLine &l = (*f.lines)[i];
		// The bytes are the file's alphabet, not the terminal's: the same
		// byte->cell map the buffer uses, so an ISO-8859 answer is readable and
		// a control byte is visible rather than swallowed.
		std::string shown;
		for (const char c : l.text)
			shown += glyphFor((unsigned char)c).s;
		Element e = text(truncate(shown, v.width));
		// Three kinds, three weights, and the order matters: what scedit sent is
		// DIM (it is not the engine speaking), what the engine refused is RED
		// (it is the one line on this feed that says something went wrong), and
		// everything else is plain. A feed that draws a refusal like an answer
		// is a feed on which a refusal is invisible - which is the state this
		// channel exists to end (INTENT 11.188).
		if (l.kind == FeedKind::Local)
			e = e | dim | color(Color::Cyan);
		else if (l.kind == FeedKind::Diagnostic)
			e = e | color(Color::Red);
		out.push_back(e);
	}
	return vbox({
		text(truncate(head, v.width)) | dim,
		vbox(std::move(out)) | reflect(v.feedBox),
	});
}

Element renderFrame(const EditCore &core, View &v)
{
	Elements lines;
	const std::size_t rows = (std::size_t)textRows(v);
	for (std::size_t r = 0; r < rows; ++r) {
		const std::size_t ln = v.top + r;
		if (ln >= core.document().lineCount()) {
			lines.push_back(text(""));
			continue;
		}
		lines.push_back(renderLine(core, v, ln, ln == core.cursor().line));
	}

	// The caret position goes FIRST: a long path must never be able to push the
	// one thing that changes with every key off the end of the line.
	char pos[64];
	std::snprintf(pos, sizeof(pos), "line %zu, byte %zu",
	              core.cursor().line + 1, core.cursor().col);
	// The live marker goes right after the caret position and BEFORE the path,
	// for the reason the position is first: it changes, it matters, and the tail
	// of this row is what truncation takes. Nothing is added when --tcp was not
	// given -- an editor with no live mode says nothing about one.
	std::string live;
	if (v.feed.enabled) {
		switch (v.feed.state) {
		case LinkState::Connected: live = " \xE2\x94\x82 live " + v.feed.endpoint; break;
		case LinkState::Offline:   live = " \xE2\x94\x82 live off"; break;
		case LinkState::Failed:    live = " \xE2\x94\x82 live FAILED"; break;
		}
	}
	std::string title = std::string("scedit \xE2\x94\x82 ") + pos
	                    + (core.dirty() ? "  *modified*" : "")
	                    + live
	                    + " \xE2\x94\x82 "
	                    + (core.path().empty() ? std::string("(new buffer)") : core.path());

	// The error COUNT goes first, for the same reason the caret position does on
	// the title row: it is what changes, and on a narrow terminal the tail of
	// this line is what truncation takes. It is also the only thing that tells
	// an author the pane exists.
	const std::string help =
		"F5 errors (" + std::to_string(core.errorHistory().size()) + ")"
		+ (v.feed.enabled ? std::string(" \xC2\xB7 F6 live \xC2\xB7 F7 send \xC2\xB7 F8 play"
		                                " \xC2\xB7 F9 feed") : std::string())
		+ " \xC2\xB7 Tab complete \xC2\xB7 Shift-Tab previous candidate"
		" \xC2\xB7 Ctrl-S/F2 save \xC2\xB7 Ctrl-Q/F10 quit";
	std::string status = v.status.empty() ? help : v.status;

	Elements frame;
	frame.push_back(text(truncate(title, v.width)) | bold | inverted);
	frame.push_back(separator());
	frame.push_back(vbox(std::move(lines)) | reflect(v.textBox));
	if (v.pane) {
		frame.push_back(separator());
		frame.push_back(renderPane(core, v));
	}
	if (v.feedPane) {
		frame.push_back(separator());
		frame.push_back(renderFeed(v));
	}
	frame.push_back(separator());
	frame.push_back(renderDocBar(core, v));
	frame.push_back(separator());
	frame.push_back(text(truncate(status, v.width)) | dim);
	return vbox(std::move(frame));
}

// --- input ------------------------------------------------------------------

//! A character typed on a UTF-8 terminal, as the single ISO-8859 byte the file
//! can hold. Returns false when the code point does not fit -- refused, never
//! transcoded into the buffer.
bool isoByteOf(const std::string &utf8, char &out)
{
	if (utf8.size() == 1) {
		const unsigned char c = (unsigned char)utf8[0];
		if (c >= 0x20 && c < 0x7F) { out = (char)c; return true; }
		return false;
	}
	if (utf8.size() == 2) {
		const unsigned char a = (unsigned char)utf8[0], b = (unsigned char)utf8[1];
		const unsigned cp = ((a & 0x1Fu) << 6) | (b & 0x3Fu);
		if (cp >= 0xA0 && cp <= 0xFF) { out = (char)(unsigned char)cp; return true; }
		return false;
	}
	return false;
}

// --- live mode ---------------------------------------------------------------
// Everything with a socket or a clock in it is here, and every one of these
// functions is called from a key handler except `tick`, which is the one
// clocked path (sc_tui.hpp S LIVE MODE).

long nowMs()
{
	using namespace std::chrono;
	return (long)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

//! The editor's half of live mode: the connection, and the window in which the
//! played file is watched for the engine's write-back.
struct Live {
	TcpClient client;
	Endpoint endpoint;
	bool enabled = false;
	//! A play is in flight: from the moment the command went out until the file
	//! changes, the window expires, or something else is played. Nothing about
	//! this is the engine telling us anything -- it cannot (sc_tcpclient.hpp).
	bool playing = false;
	long playStart = 0;
	long lastFileCheck = 0;
	//! Set when a write-back was seen that could NOT be taken, because the
	//! buffer was dirty. It stays set until the author resolves it, so the
	//! warning does not scroll away with the next keystroke.
	bool writeBackHeld = false;
};

//! The absolute path the engine must be given: it opens the file itself, and
//! its working directory is not the editor's.
std::string absolutePath(const std::string &p)
{
	char buf[PATH_MAX];
	if (::realpath(p.c_str(), buf) != nullptr)
		return std::string(buf);
	return p;
}

//! Take the engine's write-back into a CLEAN buffer, or refuse to and say so.
//! Returns the status line.
std::string takeWriteBack(EditCore &core, Live &live, View &view)
{
	if (core.dirty()) {
		live.writeBackHeld = true;
		return "spacecrafter rewrote this file (its `#!` findings) while you have unsaved "
		       "edits. Ctrl-U reloads it and your edits go; Ctrl-S twice saves over it and "
		       "the engine's findings go. Nothing has happened yet.";
	}
	std::string err;
	if (!core.reloadFromDisk(err))
		return "spacecrafter rewrote this file but it cannot be re-read: " + err;
	live.writeBackHeld = false;
	view.pane = true;   // the findings are the reason the file changed: show them
	const std::size_t tails = core.engineTailCount();
	return tails == 0
	               ? std::string("spacecrafter rewrote this file and left no findings: the "
	                             "tails it had written are cleared")
	               : "spacecrafter rewrote this file: " + std::to_string(tails) +
	                         " `#!` finding(s) \xe2\x80\x94 F3 walks them";
}

//! The one clocked path. Drains the socket, and -- at most once a second, only
//! while a play is in flight, and only for the stated window -- reads the played
//! file to see whether the engine has rewritten it.
void tick(EditCore &core, Live &live, View &view)
{
	if (!live.enabled)
		return;
	live.client.poll();
	if (!live.playing)
		return;
	const long now = nowMs();
	if (now - live.lastFileCheck < kFileCheckMs)
		return;
	live.lastFileCheck = now;
	switch (core.diskState()) {
	case DiskState::Changed:
		// The write-back HAS happened; that is also the only end-of-run signal
		// this editor can observe, so the window closes here.
		live.playing = false;
		view.status = takeWriteBack(core, live, view);
		return;
	case DiskState::Gone:
		live.playing = false;
		view.status = "the file this buffer came from can no longer be read";
		return;
	case DiskState::Same:
	case DiskState::NoFile:
		break;
	}
	if (now - live.playStart > kPlayWindowMs) {
		live.playing = false;
		view.status = "no write-back after 5 minutes \xe2\x80\x94 scedit has stopped watching the file. "
		              "It is still compared before every save.";
	}
}

//! Copy the link's state into the render's view. Called once per frame, so the
//! renderer reads a snapshot and never a live socket.
void syncFeedView(View &view, Live &live)
{
	view.feed.enabled = live.enabled;
	view.feed.endpoint = live.endpoint.text();
	view.feed.state = live.client.state();
	view.feed.dropped = live.client.dropped();
	view.feed.sent = live.client.linesSent();
	view.feed.bytes = live.client.bytesIn();
	view.feed.playing = live.playing;
	view.feed.lines = &live.client.feed();
}

} // namespace

// ---------------------------------------------------------------------------

int runEditor(const std::string &grammarPath, const std::string &file, const LiveOptions &opts)
{
	EditCore core;
	std::string err;
	if (!core.open(grammarPath, file, err)) {
		std::fprintf(stderr, "scedit: %s\n", err.c_str());
		return 2;
	}

	View view;
	Live live;
	live.enabled = opts.enabled;
	live.endpoint = opts.endpoint;
	view.feed.enabled = opts.enabled;
	view.feed.endpoint = opts.endpoint.text();
	bool quitPending = false;
	//! A save that was refused because the file changed under it. The second
	//! Ctrl-S is the explicit "my edits win" -- same shape as the quit warning
	//! above it, and the refusal message names both ways out before it.
	bool overwritePending = false;
	auto screen = ScreenInteractive::Fullscreen();
	screen.TrackMouse(true);

	// FTXUI's raw mode clears ICANON and ECHO but leaves IXON set
	// (screen_interactive.cpp:587-597), so Ctrl-S and Ctrl-Q would be eaten by
	// the terminal's flow control before any component sees them. Clear it for
	// the duration of the editor and put it back afterwards; F2 and F10 are
	// bound to the same two actions for terminals where this does not take.
	termios saved{};
	const bool haveTty = ::tcgetattr(STDIN_FILENO, &saved) == 0;
	if (haveTty) {
		termios raw = saved;
		raw.c_iflag &= ~(tcflag_t)IXON;
		::tcsetattr(STDIN_FILENO, TCSANOW, &raw);
	}

	// CTRL+<letter> arrives as the control code itself (event.hpp:25).
	const Event kCtrlS = Event::Special(std::string(1, (char)19));
	const Event kCtrlQ = Event::Special(std::string(1, (char)17));
	const Event kCtrlC = Event::Special(std::string(1, (char)3));
	// The error pane's three actions, each with a control-code twin for
	// terminals that eat function keys.
	const Event kCtrlE = Event::Special(std::string(1, (char)5));    // toggle
	const Event kCtrlN = Event::Special(std::string(1, (char)14));   // next
	const Event kCtrlP = Event::Special(std::string(1, (char)16));   // previous
	// Live mode's, each with a control-code twin for the same reason.
	const Event kCtrlT = Event::Special(std::string(1, (char)20));   // connect/disconnect
	const Event kCtrlL = Event::Special(std::string(1, (char)12));   // send this line
	const Event kCtrlR = Event::Special(std::string(1, (char)18));   // run (play) this file
	const Event kCtrlW = Event::Special(std::string(1, (char)23));   // show/hide the feed
	const Event kCtrlB = Event::Special(std::string(1, (char)2));    // feed: older
	const Event kCtrlF = Event::Special(std::string(1, (char)6));    // feed: newer
	const Event kCtrlU = Event::Special(std::string(1, (char)21));   // reload from disk

	auto renderer = Renderer([&] {
		// Terminal::Size(), NOT screen.dimx()/dimy(): a ScreenInteractive learns
		// its size from the document it has just laid out, so during the FIRST
		// render those are still zero -- and a text pane laid out one row tall
		// makes every mouse click below row one land nowhere.
		const Dimensions d = Terminal::Size();
		view.width = d.dimx;
		view.height = d.dimy;
		syncFeedView(view, live);
		if (view.follow)
			scrollToCursor(core, view);
		return renderFrame(core, view);
	});

	auto app = CatchEvent(renderer, [&](Event e) {
		const std::string keep = view.status;
		// The clocked event is not an action: it must not clear the status the
		// author is reading, must not move the view, and must not cancel a
		// pending confirmation.
		if (e == Event::Custom) {
			tick(core, live, view);
			if (view.status.empty())
				view.status = keep;
			return true;
		}
		view.status.clear();
		view.follow = true;   // any deliberate action brings the caret back

		if (e.is_mouse()) {
			const Mouse &m = e.mouse();
			// The wheel over the FEED scrolls the feed: the pointer is on it, and
			// scrolling the text under a pointer that is somewhere else is the
			// behaviour nobody means.
			const bool onFeed = view.feedPane && m.y >= view.feedBox.y_min
			                   && m.y <= view.feedBox.y_max;
			if (onFeed && (m.button == Mouse::WheelUp || m.button == Mouse::WheelDown)) {
				const std::size_t n = live.client.feed().size();
				const std::size_t cap = n > (std::size_t)kFeedRows ? n - (std::size_t)kFeedRows : 0;
				if (m.button == Mouse::WheelUp)
					view.feed.back = std::min(cap, view.feed.back + 3);
				else
					view.feed.back = view.feed.back >= 3 ? view.feed.back - 3 : 0;
				view.status = keep;
				return true;
			}
			if (m.button == Mouse::WheelUp) {
				view.top = view.top >= 3 ? view.top - 3 : 0;
				view.follow = false;
				return true;
			}
			if (m.button == Mouse::WheelDown) {
				const std::size_t last = core.document().lineCount() - 1;
				view.top = std::min(view.top + 3, last);
				view.follow = false;
				return true;
			}
			if (m.button == Mouse::Left && m.motion == Mouse::Pressed) {
				// The reflected box is where the text pane REALLY is on the
				// screen the user just clicked on.
				const int drawn = view.textBox.y_max - view.textBox.y_min + 1;
				const int dy = m.y - view.textBox.y_min;
				const int dx = m.x - view.textBox.x_min - kGutter;
				if (dy >= 0 && dy < drawn) {
					const std::size_t ln = view.top + (std::size_t)dy;
					const std::size_t col = dx <= 0 ? view.hscroll
					                                : view.hscroll + (std::size_t)dx;
					core.moveTo(ln, col);
					return true;
				}
				// A click on a pane row warps the caret to that entry -- the
				// same core call the keyboard uses, and the same reflected-box
				// arithmetic as the text above it.
				if (view.pane) {
					const int pdrawn = view.paneBox.y_max - view.paneBox.y_min + 1;
					const int pdy = m.y - view.paneBox.y_min;
					if (pdy >= 0 && pdy < pdrawn) {
						const std::size_t i = paneTop(core, view) + (std::size_t)pdy;
						if (i < core.errorHistory().size()) {
							warpToRow(core, view, i);
							return true;
						}
					}
				}
			}
			view.status = keep;
			return false;
		}

		if (e == Event::Escape || e == kCtrlQ || e == kCtrlC || e == Event::F10) {
			if (core.dirty() && !quitPending) {
				quitPending = true;
				view.status = "unsaved changes \xe2\x80\x94 Ctrl-S to save, Ctrl-Q again to discard them";
				return true;
			}
			screen.Exit();
			return true;
		}
		quitPending = false;

		if (e == kCtrlS || e == Event::F2) {
			std::string serr;
			if (overwritePending) {
				// The author was told what would be lost and pressed it again.
				overwritePending = false;
				live.writeBackHeld = false;
				view.status = core.saveOverwriting(serr)
					                      ? "saved over spacecrafter's write: " + core.path()
					                      : "NOT saved: " + serr;
				return true;
			}
			if (core.save(serr)) {
				view.status = "saved " + core.path();
				return true;
			}
			// The one refusal that has a way through: the file changed under us.
			// Ctrl-S again takes it, and the message says what that costs.
			if (core.diskState() == DiskState::Changed) {
				overwritePending = true;
				view.status = "NOT saved: " + serr + "  [Ctrl-U reload \xC2\xB7 Ctrl-S again to save anyway]";
				return true;
			}
			view.status = "NOT saved: " + serr;
			return true;
		}
		overwritePending = false;
		// The error pane (scedit/INTENT.md S5 item 15(a-ii)). F3/F4 open it as
		// well as move in it: an author who wants the next error should not
		// have to know the pane exists first.
		if (e == Event::F5 || e == kCtrlE) {
			view.pane = !view.pane;
			return true;
		}
		if (e == Event::F3 || e == kCtrlN || e == Event::F4 || e == kCtrlP) {
			if (core.errorHistory().empty()) {
				view.status = "no errors: scedit finds nothing here, and spacecrafter left no #! tail";
				return true;
			}
			view.pane = true;
			warpStep(core, view, (e == Event::F3 || e == kCtrlN) ? +1 : -1);
			return true;
		}
		// --- live mode -------------------------------------------------
		// Every one of these is bound ONLY when --tcp was given, so an editor
		// without live mode cannot reach a socket by a mis-typed key.
		if (live.enabled && (e == Event::F6 || e == kCtrlT)) {
			if (live.client.connected()) {
				live.client.disconnect();
				view.status = "disconnected from " + live.endpoint.text();
			} else {
				std::string cerr;
				view.feedPane = true;
				view.status = live.client.connect(live.endpoint, cerr)
					                      ? "connected to " + live.endpoint.text()
					                      : cerr;
			}
			return true;
		}
		if (live.enabled && (e == Event::F7 || e == kCtrlL)) {
			if (!live.client.connected()) {
				view.status = "not connected \xe2\x80\x94 F6 connects to " + live.endpoint.text();
				return true;
			}
			// The line as the author wrote it. The comment cut is the ENGINE's
			// (parseCommand does it on every channel), so scedit sends the bytes
			// and does not pre-chew them -- but it does refuse to send a line the
			// engine would read as no command at all, which is scedit's own
			// reading of that same rule and saves a pointless round trip.
			const std::string raw = core.document().line(core.cursor().line);
			if (!core.currentLine().has_command) {
				view.status = "this line is not a command the engine would run "
					              "(blank, or a comment): nothing was sent";
				return true;
			}
			std::string serr;
			view.feedPane = true;
			view.feed.back = 0;   // anything you send brings the feed back to now
			view.status = live.client.send(raw, serr)
				                      ? "sent line " + std::to_string(core.cursor().line + 1)
				                      : "NOT sent: " + serr;
			return true;
		}
		if (live.enabled && (e == Event::F8 || e == kCtrlR)) {
			if (!live.client.connected()) {
				view.status = "not connected \xe2\x80\x94 F6 connects to " + live.endpoint.text();
				return true;
			}
			if (!core.hasFile()) {
				view.status = "there is no file to play: the engine opens the file "
					              "itself, so this buffer must be saved somewhere first";
				return true;
			}
			// The engine reads the FILE, not this buffer, so what is on disk must
			// be what the author is looking at. A save that cannot happen stops
			// the play, with the save's own reason: playing the old bytes and
			// reporting them as this file's is exactly the confusion to avoid.
			if (core.dirty()) {
				std::string serr;
				if (!core.save(serr)) {
					if (core.diskState() == DiskState::Changed)
						overwritePending = true;
					view.status = "not played, because it could not be saved first: " + serr;
					return true;
				}
			}
			const std::string abs = absolutePath(core.path());
			std::string serr;
			if (!live.client.send("script action play filename " + abs, serr)) {
				view.status = "NOT played: " + serr;
				return true;
			}
			live.playing = true;
			live.playStart = nowMs();
			live.lastFileCheck = live.playStart;
			view.feedPane = true;
			view.feed.back = 0;
			view.status = "playing " + abs + " \xe2\x80\x94 the engine says nothing when a "
				              "script ends, so scedit now watches this file for the "
				              "`#!` findings it writes at the end of a run";
			return true;
		}
		if (live.enabled && (e == Event::F9 || e == kCtrlW)) {
			view.feedPane = !view.feedPane;
			return true;
		}
		if (live.enabled && (e == Event::F11 || e == kCtrlB || e == Event::F12 || e == kCtrlF)) {
			view.feedPane = true;
			const std::size_t n = live.client.feed().size();
			const std::size_t cap = n > (std::size_t)kFeedRows ? n - (std::size_t)kFeedRows : 0;
			if (e == Event::F11 || e == kCtrlB)
				view.feed.back = std::min(cap, view.feed.back + (std::size_t)kFeedRows);
			else
				view.feed.back = view.feed.back >= (std::size_t)kFeedRows
					                         ? view.feed.back - (std::size_t)kFeedRows : 0;
			return true;
		}
		// Reload -- the other half of the write-back choice, and useful on its own
		// whenever something else has written the file.
		if (e == kCtrlU) {
			std::string rerr;
			const bool wasDirty = core.dirty();
			if (!core.reloadFromDisk(rerr)) {
				view.status = "NOT reloaded: " + rerr;
				return true;
			}
			live.writeBackHeld = false;
			const std::size_t tails = core.engineTailCount();
			view.status = std::string("reloaded ") + core.path()
				              + (wasDirty ? " \xe2\x80\x94 the unsaved edits in this buffer are gone" : "")
				              + (tails ? ", " + std::to_string(tails) + " `#!` finding(s) from spacecrafter"
					                       : "");
			if (tails)
				view.pane = true;
			return true;
		}
		if (e == Event::Tab) {
			if (!core.acceptCompletion() && core.completion().candidates.size() > 1)
				view.status = "candidate " + std::to_string(core.completion().selected + 1)
				              + " of " + std::to_string(core.completion().candidates.size());
			return true;
		}
		if (e == Event::TabReverse) {
			core.cycleCompletion(-1);
			return true;
		}
		if (e == Event::ArrowLeft) { core.moveLeft(); return true; }
		if (e == Event::ArrowRight) { core.moveRight(); return true; }
		if (e == Event::ArrowUp) { core.moveUp(); return true; }
		if (e == Event::ArrowDown) { core.moveDown(); return true; }
		if (e == Event::Home) { core.moveHome(); return true; }
		if (e == Event::End) { core.moveEnd(); return true; }
		if (e == Event::PageUp) { core.moveUp((std::size_t)textRows(view)); return true; }
		if (e == Event::PageDown) { core.moveDown((std::size_t)textRows(view)); return true; }
		if (e == Event::Backspace) { core.backspace(); return true; }
		if (e == Event::Delete) { core.del(); return true; }
		if (e == Event::Return) { core.insertNewline(); return true; }

		if (e.is_character()) {
			char b = 0;
			if (isoByteOf(e.character(), b)) {
				core.insertText(std::string(1, b));
			} else {
				view.status = "this file is ISO-8859 bytes: '" + e.character()
				              + "' has no byte here, so it was not inserted";
			}
			return true;
		}
		view.status = keep;
		return false;
	});

	// THE CLOCK, and there is exactly one. It runs only in live mode, it only
	// posts an event, and the handler decides what that event means (drain the
	// socket; at most once a second, and only inside a play window, read the
	// file). Without --tcp this thread is never started, so an editor with no
	// live mode has no timer in it at all.
	std::atomic<bool> stopTicker{false};
	std::thread ticker;
	if (live.enabled) {
		ticker = std::thread([&] {
			while (!stopTicker.load()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(kDrainMs));
				if (stopTicker.load())
					break;
				if (live.client.connected() || live.playing)
					screen.PostEvent(Event::Custom);
			}
		});
	}

	screen.Loop(app);

	stopTicker.store(true);
	if (ticker.joinable())
		ticker.join();
	// The socket is closed politely rather than by the destructor, so the engine
	// sees the $LOGOFF it was told to expect.
	if (live.client.connected())
		live.client.disconnect();
	if (haveTty)
		::tcsetattr(STDIN_FILENO, TCSANOW, &saved);
	return 0;
}

// ---------------------------------------------------------------------------

int uiSelfTest(const std::string &grammarPath)
{
	struct Case {
		const char *name;
		const char *buffer;
		std::size_t line;
		long col;      //!< -1 = end of that line
		bool pane = false;   //!< draw the error pane
		//! >= 0: press F3 this many times before rendering -- the pane's own
		//! action, so what the record pins is where a WARP leaves the caret.
		int warps = 0;
		//! The pane costs kPaneChrome rows; the pane cases get a taller screen
		//! rather than one text row. Every other frame keeps 14, so the record
		//! of the frames that existed before the pane is unchanged by it.
		int height = 14;
		//! Live mode. `feedLines` is a canned conversation -- the pane is drawn
		//! from a deque, and here that deque is a literal rather than a socket,
		//! which is the whole reason the renderer takes data and not a client.
		bool live = false;
		LinkState link = LinkState::Connected;
		int feedBack = 0;
		std::size_t dropped = 0;
		std::vector<FeedLine> feedLines = {};
	};
	// Each case exists to put ONE claim of the D31 spec on a real screen.
	static const Case cases[] = {
		{"ghost-command", "zo", 0, -1},
		{"ghost-empty-value", "flag stars ", 0, -1},
		{"ghost-enumerated-value", "date load c", 0, -1},
		{"doc-set-name", "set star_scale 2", 0, 10},
		{"open-key-list", "body ac", 0, -1},
		{"finding-invisible-separator", "body name Earth albedo\xA0" "1", 0, 0},
		{"no-doc-extracted", "suntrace sun Earth", 0, 10},
		// ISO-8859 bytes on a UTF-8 screen, and a CRLF file: the 0xE9 shows as
		// the letter it is, the tab as a marker, and NO line-ending marker
		// appears, because the '\r' is the terminator and not the text.
		{"iso-8859-and-crlf", "flag stars on\r\nbody name Caf\xE9" "\tx\r\n", 1, 0},
		// The comment after a '#': dim from the '#' to the end of the line, no
		// finding (parse_model.comments.mid_line -- the engine reads none of it).
		{"comment-tail", "media action pause # stop video", 0, 0},
		// The caret inside that comment: no ghost, and the bar says "comment".
		{"caret-in-comment", "media action pause # stop video", 0, 25},
		// An opener never closed is reported at the OPENER line (the root),
		// although the checker only knows at the end of the file.
		{"finding-unclosed-struct", "struct if a equal b\nflag stars on\n", 0, 0},
		// A `#!` tail the engine wrote (parse_model.comments.machine_tail): dim
		// like any comment, and row 4 says what the engine wrote and that
		// scedit's own finding agrees.
		{"machine-tail", "struct if end #! this 'struct if end' closes nothing: no 'struct if' is open here\n", 0, 0},
		// The same tail on a line scedit finds clean: the relation says so.
		{"machine-tail-stale", "flag stars on #! this 'struct if end' closes nothing: no 'struct if' is open here\n", 0, 0},
		// The error pane (scedit/INTENT.md S5 item 15(a-ii)). Both sources
		// mixed: line 2 carries an engine tail AND scedit's own finding -- two
		// rows, the engine's first -- line 3 a stale tail alone, line 4 a
		// finding alone. The caret is on line 1, so NO row is marked.
		{"pane-mixed",
		 "flag stars on\n"
		 "struct if end #! this 'struct if end' closes nothing: no 'struct if' is open here\n"
		 "flag stars off #! this 'struct if end' closes nothing: no 'struct if' is open here\n"
		 "zomo action now\n",
		 0, 0, true, 0, 20},
		// One F3 from there: the caret warps to the FIRST entry -- line 2, at
		// the `#!` (the engine's row comes first on a line). The `inv` mask
		// pins the caret on the warped line, and the pane marks that line's
		// two rows with `>`.
		{"pane-warp",
		 "flag stars on\n"
		 "struct if end #! this 'struct if end' closes nothing: no 'struct if' is open here\n"
		 "flag stars off #! this 'struct if end' closes nothing: no 'struct if' is open here\n"
		 "zomo action now\n",
		 0, 0, true, 1, 20},
		// A second F3 steps to the SCEDIT row on the same line: same line, the
		// caret on the finding's own byte rather than on the `#!`.
		{"pane-warp-twice",
		 "flag stars on\n"
		 "struct if end #! this 'struct if end' closes nothing: no 'struct if' is open here\n"
		 "flag stars off #! this 'struct if end' closes nothing: no 'struct if' is open here\n"
		 "zomo action now\n",
		 0, 0, true, 2, 20},
		// A clean buffer: the pane is open and says so, rather than showing an
		// unexplained empty box.
		{"pane-empty", "flag stars on\nbody name Earth\n", 0, 0, true, 0, 20},
		// LIVE MODE (--tcp). A conversation: what scedit sent is dim and marked
		// `>`, what the engine sent is not -- because one of them is the engine
		// speaking and the other is not, and a feed that draws them alike is a
		// feed you cannot read. The title row and the status row gain live
		// text HERE and nowhere else: with --tcp absent, every frame above is
		// byte-identical to what it was before live mode existed.
		{"live-feed", "flag stars on\nstruct if end\n", 0, 0, false, 0, 22, true,
		 LinkState::Connected, 0, 0,
		 {{FeedKind::Local, "connected to 127.0.0.1:7805, subscribed with $LOGON"},
		  {FeedKind::Engine, "Vous receverez maintenant les logs"},
		  {FeedKind::Local, "> get status position"},
		  {FeedKind::Engine, " 45.00; 3.00;  75.00;2461233.500000;  12.500000;"},
		  {FeedKind::Local, "> flag stars on"}}},
		// A REFUSAL on the feed. The engine labels it `$DIAG|` and the pane
		// draws it red, so the one line that says something went wrong does not
		// look like the answer above it. Before INTENT 11.188 this line could
		// not exist: the refusal went to a log file on the engine's machine.
		{"live-feed-diagnostic", "flag stars on\nflagg stars on\n", 0, 0, false, 0, 22, true,
		 LinkState::Connected, 0, 0,
		 {{FeedKind::Local, "connected to 127.0.0.1:7805, subscribed with $LOGON and $DIAGON"},
		  {FeedKind::Engine, "Vous receverez maintenant les logs"},
		  {FeedKind::Engine, "$DIAGON ok: this connection now receives every diagnostic"},
		  {FeedKind::Local, "> flagg stars on"},
		  {FeedKind::Diagnostic,
		   "$DIAG|tcp#4|Unrecognized or malformed command name|flagg stars on"},
		  {FeedKind::Local, "> get status position"},
		  {FeedKind::Engine, " 45.00; 3.00;  75.00;2461233.500000;  12.500000;"}}},
		// --tcp given, nothing connected: the header says which of the two
		// reasons for silence this is, and the title says `live off`.
		{"live-offline", "flag stars on\n", 0, 0, false, 0, 22, true,
		 LinkState::Offline, 0, 0, {}},
		// Scrolled back, with lines already discarded by the bound: `+2 newer
		// below` and `3 dropped` are both on the header, because a pane that
		// hides either is showing an old line as the latest.
		{"live-feed-scrolled", "flag stars on\n", 0, 0, false, 0, 22, true,
		 LinkState::Connected, 2, 3,
		 {{FeedKind::Engine, "line one"}, {FeedKind::Engine, "line two"},
		  {FeedKind::Engine, "line three"}, {FeedKind::Engine, "line four"},
		  {FeedKind::Engine, "line five"}, {FeedKind::Engine, "line six"},
		  {FeedKind::Engine, "line seven"}}},
	};

	for (const Case &c : cases) {
		EditCore core;
		std::string err;
		if (!core.openBytes(grammarPath, "selftest.sts", c.buffer, err)) {
			std::fprintf(stderr, "scedit: %s\n", err.c_str());
			return 2;
		}
		const std::size_t col = c.col < 0 ? core.document().line(c.line).size()
		                                  : (std::size_t)c.col;
		core.moveTo(c.line, col);
		View v;
		v.width = 100;
		v.height = c.height;
		v.pane = c.pane;
		// The feed's lines live here for the length of this frame; the view
		// borrows them exactly as it borrows a TcpClient's deque (I5).
		std::deque<FeedLine> feed(c.feedLines.begin(), c.feedLines.end());
		v.feedPane = c.live;
		v.feed.enabled = c.live;
		v.feed.state = c.link;
		v.feed.lines = &feed;
		v.feed.back = (std::size_t)c.feedBack;
		v.feed.dropped = c.dropped;
		v.feed.sent = c.live && c.link == LinkState::Connected ? 2 : 0;
		v.feed.bytes = c.live && c.link == LinkState::Connected ? 96 : 0;
		// The pane's own action, through the same call the F3 key makes.
		for (int w = 0; w < c.warps; ++w)
			warpStep(core, v, +1);
		scrollToCursor(core, v);
		Element frame = renderFrame(core, v);
		Screen screen = Screen::Create(Dimension::Fixed(v.width), Dimension::Fixed(v.height));
		Render(screen, frame);

		std::printf("### %s\n", c.name);
		for (int y = 0; y < v.height; ++y) {
			std::string row;
			for (int x = 0; x < v.width; ++x)
				row += screen.PixelAt(x, y).character;
			while (!row.empty() && row.back() == ' ')
				row.pop_back();
			std::printf("| %s\n", row.c_str());
		}
		// The greyness of the ghost is the claim; prove it from the pixels.
		const int caretRow = 2 + (int)(core.cursor().line - v.top);
		std::string mask, mark, under, inv;
		for (int x = 0; x < v.width; ++x) {
			const Pixel &p = screen.PixelAt(x, caretRow);
			mask += p.dim ? 'd' : '-';
			// The look-alike-space marker: red + bold. Printed as its own mask
			// so the gate pins the COLUMN it lands on, not merely its presence.
			mark += (p.bold && p.foreground_color == Color::Red) ? 'm' : '-';
			// A finding's span, underlined at exactly its bytes.
			under += p.underlined ? 'u' : '-';
			// The caret: the standard SGR inversion, on exactly one cell.
			inv += p.inverted ? 'i' : '-';
		}
		auto rtrim = [](std::string &m) {
			while (!m.empty() && m.back() == '-')
				m.pop_back();
		};
		rtrim(mask);
		rtrim(mark);
		rtrim(under);
		rtrim(inv);
		std::printf("dim %s\n", mask.c_str());
		std::printf("mark %s\n", mark.c_str());
		std::printf("under %s\n", under.c_str());
		std::printf("inv %s\n", inv.c_str());
		// LIVE CASES ONLY, so every frame recorded before live mode existed
		// keeps its four lines exactly. One character per feed row, read off
		// the pixels of the box the renderer reflected: `L` = drawn as scedit's
		// own line (dim + cyan), `E` = drawn as the engine's, `.` = empty. This
		// is the claim "you can tell who said it", proved from the screen.
		if (c.live) {
			std::string kinds;
			for (int r = 0; r < kFeedRows; ++r) {
				const int y = v.feedBox.y_min + r;
				// The first cell that is not blank: an engine answer can begin
				// with a space (`get status position` does), and reading column
				// zero alone would call that row empty.
				char k = '.';
				for (int x = v.feedBox.x_min; x <= v.feedBox.x_max; ++x) {
					const Pixel &p = screen.PixelAt(x, y);
					if (p.character == " " || p.character.empty())
						continue;
					if (p.dim && p.foreground_color == Color::Cyan)
						k = 'L';
					else if (!p.dim && p.foreground_color == Color::Red)
						k = 'D';
					else
						k = 'E';
					break;
				}
				kinds += k;
			}
			std::printf("feed %s\n", kinds.c_str());
		}
	}
	return 0;
}

} // namespace scedit
