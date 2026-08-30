#include "sc_tui.hpp"

#include <algorithm>
#include <cstdio>
#include <termios.h>
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
//! title + separator + [text] + separator + doc bar + separator + status
constexpr int kChrome = 1 + 1 + 1 + kDocRows + 1 + 1;

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
	case Openness::Open: return "known ones — there are more";
	case Openness::Unstated: return "";
	}
	return "";
}

//! What the editor is showing, over and above what the core knows.
struct View {
	std::size_t top = 0;        //!< first buffer line drawn
	std::size_t hscroll = 0;    //!< first byte column drawn
	//! Whether the view must chase the caret. A key press turns it on; the
	//! wheel turns it off, because scrolling away from the caret is the whole
	//! point of scrolling and a view that snaps straight back has not scrolled.
	bool follow = true;
	std::string status;
	int width = 100;
	int height = 30;
	Box textBox;                //!< filled by the renderer, read by the mouse
};

int textRows(const View &v) { return std::max(1, v.height - kChrome); }
int textCols(const View &v) { return std::max(1, v.width - kGutter); }

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
	std::string row1 = d.path.empty() ? std::string("—") : d.path;
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
	if (!diags.empty()) {
		std::string t;
		for (const auto *dg : diags)
			t += (t.empty() ? "" : " | ") + dg->severity + ": " + dg->message + " [-W" + dg->id + "]";
		row4 = text(truncate(t, w)) | color(severityColor(diags.front()->severity));
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
	std::string title = std::string("scedit \xE2\x94\x82 ") + pos
	                    + (core.dirty() ? "  *modified*" : "")
	                    + " \xE2\x94\x82 "
	                    + (core.path().empty() ? std::string("(new buffer)") : core.path());

	const std::string help =
		"Tab complete \xC2\xB7 Shift-Tab previous candidate \xC2\xB7 Ctrl-S/F2 save \xC2\xB7 Ctrl-Q/F10 quit";
	std::string status = v.status.empty() ? help : v.status;

	return vbox({
		text(truncate(title, v.width)) | bold | inverted,
		separator(),
		vbox(std::move(lines)) | reflect(v.textBox),
		separator(),
		renderDocBar(core, v),
		separator(),
		text(truncate(status, v.width)) | dim,
	});
}

// --- input ------------------------------------------------------------------

//! A character typed on a UTF-8 terminal, as the single ISO-8859 byte the file
//! can hold. Returns false when the code point does not fit — refused, never
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

} // namespace

// ---------------------------------------------------------------------------

int runEditor(const std::string &grammarPath, const std::string &file)
{
	EditCore core;
	std::string err;
	if (!core.open(grammarPath, file, err)) {
		std::fprintf(stderr, "scedit: %s\n", err.c_str());
		return 2;
	}

	View view;
	bool quitPending = false;
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

	auto renderer = Renderer([&] {
		// Terminal::Size(), NOT screen.dimx()/dimy(): a ScreenInteractive learns
		// its size from the document it has just laid out, so during the FIRST
		// render those are still zero — and a text pane laid out one row tall
		// makes every mouse click below row one land nowhere.
		const Dimensions d = Terminal::Size();
		view.width = d.dimx;
		view.height = d.dimy;
		if (view.follow)
			scrollToCursor(core, view);
		return renderFrame(core, view);
	});

	auto app = CatchEvent(renderer, [&](Event e) {
		const std::string keep = view.status;
		view.status.clear();
		view.follow = true;   // any deliberate action brings the caret back

		if (e.is_mouse()) {
			const Mouse &m = e.mouse();
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
			}
			view.status = keep;
			return false;
		}

		if (e == Event::Escape || e == kCtrlQ || e == kCtrlC || e == Event::F10) {
			if (core.dirty() && !quitPending) {
				quitPending = true;
				view.status = "unsaved changes — Ctrl-S to save, Ctrl-Q again to discard them";
				return true;
			}
			screen.Exit();
			return true;
		}
		quitPending = false;

		if (e == kCtrlS || e == Event::F2) {
			std::string serr;
			view.status = core.save(serr) ? "saved " + core.path() : "NOT saved: " + serr;
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

	screen.Loop(app);
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
		// finding (parse_model.comments.mid_line — the engine reads none of it).
		{"comment-tail", "media action pause # stop video", 0, 0},
		// The caret inside that comment: no ghost, and the bar says "comment".
		{"caret-in-comment", "media action pause # stop video", 0, 25},
		// An opener never closed is reported at the OPENER line (the root),
		// although the checker only knows at the end of the file.
		{"finding-unclosed-struct", "struct if a equal b\nflag stars on\n", 0, 0},
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
		v.height = 14;
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
	}
	return 0;
}

} // namespace scedit
