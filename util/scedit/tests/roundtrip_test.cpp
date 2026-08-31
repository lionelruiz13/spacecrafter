/*
 * scedit -- the round-trip gate's worker.
 *
 * WHY THIS EXISTS
 * ===============
 * These files are ISO-8859 and their bytes carry meaning: 0xA0 in a column is
 * the difference between two words and one (`invisible-separator`), a CRLF file
 * has a '\r' at the end of every line, and the last line may have no terminator
 * at all. An editor that "helpfully" normalises any of that rewrites a show
 * before anyone notices. So the promise is exact: OPEN AND SAVE CHANGES NOTHING,
 * and an edit changes only the line it was made on.
 *
 * Leg 1 is checked by the caller comparing MD5s (tests/roundtrip_gate.cmake).
 * Leg 2 is checked here, because "the file differs by exactly this insertion"
 * is not a checksum question.
 *
 *   roundtrip_test <grammar.json> <in> <out-unedited> <out-edited>
 */

#include "sc_editcore.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

using namespace scedit;

namespace {

bool readAll(const std::string &path, std::string &out)
{
	std::ifstream in(path, std::ios::binary);
	if (!in)
		return false;
	std::ostringstream b;
	b << in.rdbuf();
	out = b.str();
	return true;
}

} // namespace

int main(int argc, char **argv)
{
	if (argc < 5) {
		std::fprintf(stderr, "usage: roundtrip_test <grammar> <in> <out-unedited> <out-edited>\n");
		return 2;
	}
	const std::string grammar = argv[1], in = argv[2], out1 = argv[3], out2 = argv[4];

	std::string original;
	if (!readAll(in, original)) {
		std::fprintf(stderr, "roundtrip: cannot read %s\n", in.c_str());
		return 2;
	}

	int failures = 0;

	// --- leg 1: open, touch nothing, save --------------------------------
	{
		EditCore core;
		std::string err;
		if (!core.open(grammar, in, err)) {
			std::fprintf(stderr, "roundtrip: %s\n", err.c_str());
			return 2;
		}
		if (core.dirty()) {
			std::printf("  FAIL  opening a file marks it modified\n");
			++failures;
		}
		if (core.document().bytes() != original) {
			std::printf("  FAIL  the buffer is not the file's bytes (%zu vs %zu)\n",
			            core.document().bytes().size(), original.size());
			++failures;
		}
		if (!core.saveAs(out2.empty() ? out1 : out1, err)) {
			std::fprintf(stderr, "roundtrip: %s\n", err.c_str());
			return 2;
		}
		std::printf("  leg 1: %s -> %s, %zu bytes, %zu lines\n", in.c_str(), out1.c_str(),
		            original.size(), core.document().lineCount());
	}

	// --- leg 2: one edit, and only that edit ------------------------------
	{
		EditCore core;
		std::string err;
		if (!core.open(grammar, in, err)) {
			std::fprintf(stderr, "roundtrip: %s\n", err.c_str());
			return 2;
		}
		const std::size_t line = core.document().lineCount() > 100
		                         ? 100 : core.document().lineCount() / 2;
		core.moveTo(line, 0);
		core.insertText("X");
		if (!core.saveAs(out2, err)) {
			std::fprintf(stderr, "roundtrip: %s\n", err.c_str());
			return 2;
		}

		// Where that byte must land: after every byte of every earlier line,
		// terminators included.
		std::size_t offset = 0;
		for (std::size_t i = 0; i < line; ++i) {
			// Recomputed from the saved buffer, but the ORIGINAL is the
			// comparison subject below, so a bug in the buffer cannot hide.
			const std::size_t nl = original.find('\n', offset);
			if (nl == std::string::npos) {
				offset = original.size();
				break;
			}
			offset = nl + 1;
		}
		std::string want = original;
		want.insert(offset, "X");

		std::string got;
		if (!readAll(out2, got)) {
			std::fprintf(stderr, "roundtrip: cannot read %s\n", out2.c_str());
			return 2;
		}
		if (got != want) {
			std::printf("  FAIL  an edit on line %zu changed more than that line\n", line + 1);
			std::size_t i = 0;
			while (i < got.size() && i < want.size() && got[i] == want[i])
				++i;
			std::printf("        first difference at byte %zu (sizes %zu vs %zu)\n",
			            i, got.size(), want.size());
			++failures;
		} else {
			std::printf("  leg 2: one byte inserted at line %zu -> file offset %zu, "
			            "every other byte identical\n", line + 1, offset);
		}
	}

	std::printf("%s\n", failures ? "ROUND TRIP FAILED" : "round trip ok");
	return failures ? 1 : 0;
}
