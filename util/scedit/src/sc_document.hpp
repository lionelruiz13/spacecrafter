/*
 * scedit -- sc_document.hpp
 *
 * WHAT THIS IS FOR
 * ================
 * The edited file, held as BYTES, so that saving a file you did not change
 * gives back the file you opened -- byte for byte -- and saving one you changed
 * on line 40 leaves lines 1-39 and 41-end exactly as their author wrote them.
 *
 * WHY THAT IS A REQUIREMENT AND NOT A NICETY
 * ==========================================
 * These files are ISO-8859, not UTF-8, and their bytes are significant:
 * `--check`'s `invisible-separator` rule exists because byte 0xA0 in column N
 * changes what the engine reads (grammar/sc-grammar.json lint_seeds). A round
 * trip through any decoder -- UTF-8, latin-1-to-UTF-8, "clean up the whitespace"
 * -- destroys the very defect the tool was written to show you, and silently
 * rewrites a file the dome will read tonight. So: no decoding, ever. A line is
 * a std::string of bytes; the editor moves a cursor over byte offsets; the
 * renderer is the only place that must think about how a byte LOOKS.
 *
 * THE LINE MODEL
 * ==============
 * Split on '\n', and a '\r' immediately before that '\n' belongs to the
 * TERMINATOR, not to the text: a line carries its own terminator bytes,
 * "\r\n" or "\n", or "" for a final line the file did not terminate. Three
 * things follow, all of them wanted:
 *   - `bytes()` is exact, whatever mixture of terminators a file holds;
 *   - the caret can never sit between the text and its '\r', and End means the
 *     end of what the author wrote;
 *   - a line opened in a CRLF file gets a CRLF line under it, because
 *     `splitLine` gives the new line the terminator the split one had. The
 *     editor does not decide what a line ending is; the file does.
 * A '\r' anywhere ELSE stays in the text -- it is not a terminator to the engine
 * either (Script::loadInternal's std::getline splits on '\n' only).
 *
 * The engine reads the line WITH its '\r' (that is why the script layer tests
 * `line[0] != '\r'`), so a consumer that must reproduce the engine's reading
 * exactly appends it back: `EditCore` does, and the tokenizer therefore sees
 * the engine's own bytes. Buffer line i is still diagnostic line i+1.
 *
 * OWNERSHIP: by value; references returned by `line()` die with the Document or
 * with the next mutation.
 */

#ifndef SCEDIT_SC_DOCUMENT_HPP
#define SCEDIT_SC_DOCUMENT_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace scedit {

class Document {
public:
	//! An empty document is ONE empty line with no terminator: an editor always
	//! has a line to put the cursor on, and `bytes()` still returns "".
	Document();

	//! Never transcodes, never normalises. `bytes()` returns its argument.
	static Document fromBytes(const std::string &bytes);

	//! Reads the file as binary. Sets `err` and returns false on I/O failure.
	bool loadFile(const std::string &path, std::string &err);
	//! Writes `bytes()` as binary. Sets `err` and returns false on I/O failure.
	//! Clears the dirty flags on success.
	bool saveFile(const std::string &path, std::string &err);

	//! The file as it would be written right now.
	std::string bytes() const;

	std::size_t lineCount() const { return lines_.size(); }
	//! Line text without its terminator.
	const std::string &line(std::size_t i) const;
	//! The terminator bytes of line i: "\r\n", "\n", or "" (unterminated last).
	const std::string &terminator(std::size_t i) const;
	//! Line i as the ENGINE reads it: the text with its '\r' put back, so a
	//! tokenizer sees the bytes std::getline would have handed the parser.
	std::string engineLine(std::size_t i) const;

	//! True when this line's bytes have been through a mutation. A false here is
	//! the round-trip guarantee: the line's bytes are the ones that were read.
	bool touched(std::size_t i) const;
	bool dirty() const { return dirty_; }

	// --- mutation (byte-level; the caller owns cursor arithmetic) -----------

	void setLine(std::size_t i, const std::string &text);
	//! Insert `text` at byte offset `col` of line i. Offsets past the end clamp.
	void insert(std::size_t i, std::size_t col, const std::string &text);
	//! Erase `n` bytes at byte offset `col` of line i. Clamps at end of line.
	void erase(std::size_t i, std::size_t col, std::size_t n);
	//! Split line i at byte offset `col`; the tail becomes line i+1 and keeps
	//! the original terminator, the head takes "\n".
	void splitLine(std::size_t i, std::size_t col);
	//! Append line i+1 to line i (the Backspace-at-column-0 move). No-op on the
	//! last line. Line i takes line i+1's terminator.
	void joinLine(std::size_t i);

private:
	struct L {
		std::string text;
		std::string term;
		bool touched = false;
	};
	std::vector<L> lines_;
	//! What a NEW line's terminator is: the one this file already uses. Never a
	//! guess -- it is read off the file, and only falls back to "\n" for a file
	//! that has no terminated line to learn from.
	std::string default_term_ = "\n";
	bool dirty_ = false;

	void touch(std::size_t i);
};

} // namespace scedit

#endif // SCEDIT_SC_DOCUMENT_HPP
