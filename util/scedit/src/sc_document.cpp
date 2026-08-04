#include "sc_document.hpp"

#include <fstream>
#include <sstream>

namespace scedit {

Document::Document()
{
	lines_.push_back(L{"", "", false});
}

Document Document::fromBytes(const std::string &bytes)
{
	Document d;
	d.lines_.clear();
	bool sawTerm = false;
	std::size_t start = 0;
	while (start <= bytes.size()) {
		const std::size_t nl = bytes.find('\n', start);
		if (nl == std::string::npos) {
			// Trailing bytes with no terminator. When the file ended ON a '\n'
			// (start == size) there is no such line — the same rule as
			// splitScriptLines, so line i here is line i+1 in a diagnostic.
			if (start < bytes.size())
				d.lines_.push_back(L{bytes.substr(start), "", false});
			break;
		}
		std::size_t end = nl;
		std::string term = "\n";
		if (end > start && bytes[end - 1] == '\r') {
			--end;
			term = "\r\n";
		}
		if (!sawTerm) {
			d.default_term_ = term;   // a new line joins the file it is in
			sawTerm = true;
		}
		d.lines_.push_back(L{bytes.substr(start, end - start), term, false});
		start = nl + 1;
	}
	if (d.lines_.empty())
		d.lines_.push_back(L{"", "", false});
	return d;
}

bool Document::loadFile(const std::string &path, std::string &err)
{
	std::ifstream in(path, std::ios::binary);
	if (!in) {
		err = "cannot open " + path;
		return false;
	}
	std::ostringstream buf;
	buf << in.rdbuf();
	if (in.bad()) {
		err = "read error on " + path;
		return false;
	}
	*this = fromBytes(buf.str());
	return true;
}

bool Document::saveFile(const std::string &path, std::string &err)
{
	const std::string out = bytes();
	std::ofstream o(path, std::ios::binary | std::ios::trunc);
	if (!o) {
		err = "cannot write " + path;
		return false;
	}
	o.write(out.data(), (std::streamsize)out.size());
	o.flush();
	if (!o) {
		err = "write error on " + path;
		return false;
	}
	for (auto &l : lines_)
		l.touched = false;
	dirty_ = false;
	return true;
}

std::string Document::bytes() const
{
	std::size_t n = 0;
	for (const auto &l : lines_)
		n += l.text.size() + l.term.size();
	std::string out;
	out.reserve(n);
	for (const auto &l : lines_) {
		out += l.text;
		out += l.term;
	}
	return out;
}

const std::string &Document::line(std::size_t i) const
{
	static const std::string empty;
	return i < lines_.size() ? lines_[i].text : empty;
}

const std::string &Document::terminator(std::size_t i) const
{
	static const std::string empty;
	return i < lines_.size() ? lines_[i].term : empty;
}

std::string Document::engineLine(std::size_t i) const
{
	if (i >= lines_.size())
		return std::string();
	// std::getline stops at '\n' and keeps everything before it, '\r' included.
	return lines_[i].term == "\r\n" ? lines_[i].text + "\r" : lines_[i].text;
}

bool Document::touched(std::size_t i) const
{
	return i < lines_.size() && lines_[i].touched;
}

void Document::touch(std::size_t i)
{
	if (i < lines_.size())
		lines_[i].touched = true;
	dirty_ = true;
}

void Document::setLine(std::size_t i, const std::string &text)
{
	if (i >= lines_.size())
		return;
	if (lines_[i].text == text)
		return;   // an assignment that changes no byte is not an edit
	lines_[i].text = text;
	touch(i);
}

void Document::insert(std::size_t i, std::size_t col, const std::string &text)
{
	if (i >= lines_.size() || text.empty())
		return;
	std::string &t = lines_[i].text;
	if (col > t.size())
		col = t.size();
	t.insert(col, text);
	touch(i);
}

void Document::erase(std::size_t i, std::size_t col, std::size_t n)
{
	if (i >= lines_.size() || n == 0)
		return;
	std::string &t = lines_[i].text;
	if (col >= t.size())
		return;
	if (col + n > t.size())
		n = t.size() - col;
	t.erase(col, n);
	touch(i);
}

void Document::splitLine(std::size_t i, std::size_t col)
{
	if (i >= lines_.size())
		return;
	L &head = lines_[i];
	if (col > head.text.size())
		col = head.text.size();
	L tail;
	tail.text = head.text.substr(col);
	tail.term = head.term;      // the file's "no final newline" property travels
	tail.touched = true;
	head.text.erase(col);
	// The new line ending is the one this line already had — so a CRLF file
	// stays a CRLF file. An unterminated last line has none to give, and takes
	// the file's own.
	head.term = head.term.empty() ? default_term_ : head.term;
	touch(i);
	lines_.insert(lines_.begin() + (long)i + 1, tail);
}

void Document::joinLine(std::size_t i)
{
	if (i + 1 >= lines_.size())
		return;
	lines_[i].text += lines_[i + 1].text;
	lines_[i].term = lines_[i + 1].term;
	lines_.erase(lines_.begin() + (long)i + 1);
	touch(i);
}

} // namespace scedit
