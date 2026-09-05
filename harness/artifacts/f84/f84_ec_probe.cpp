// F84 probe: reproduce call_system.cpp:130-160's branch expression verbatim
// and drive it with a copy that SUCCEEDS and a copy that FAILS.
#include <filesystem>
#include <iostream>
#include <string>
static void arm(const std::string& label, const std::string& src, const std::string& dst)
{
    std::error_code ec{};
    std::filesystem::create_directories(dst);
    std::filesystem::copy(src, dst, std::filesystem::copy_options::recursive, ec);
    std::cout << label
              << "\n  ec (bool)      = " << (ec ? "true (COPY FAILED)" : "false (copy ok)")
              << "\n  ec.message()   = \"" << ec.message() << "\""
              << "\n  branch taken   = "
              << ((ec || ec.message() == "Success") ? "\"Completed copy of ...\""
                                                    : "\"Failed to copy ... / Abort !\"")
              << "\n";
}
int main()
{
    arm("A. source EXISTS (copy succeeds)", "/usr/local/share/spacecrafter/data/ftp",
        "/home/claude/sc-f84/ecprobe/ok");
    arm("B. source ABSENT (copy fails)", "/usr/local/share/spacecrafter/data/no_such_class",
        "/home/claude/sc-f84/ecprobe/bad");
    return 0;
}
