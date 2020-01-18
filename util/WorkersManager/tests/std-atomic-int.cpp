#include <atomic>

int main() {
	std::atomic<int> i;
	for(i = 0; i < 1000000; ++i);
}
