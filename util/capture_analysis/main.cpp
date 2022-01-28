#include "../../src/EntityCore/Tools/CaptureMetrics.hpp"
#include "../../src/capture.hpp"
#include <iostream>

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        std::cerr << "Capture file is expected as argument.\n";
        return -1;
    }
    CaptureMetrics capture(argv[1], CAPTURE_FLAG_NAMES);
    capture.analyze();
    capture.display();
    return 0;
}
