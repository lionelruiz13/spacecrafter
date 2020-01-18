#include "partition-copy.hpp"

int main(int argc, char **argv) {
	PartitionCopy* pc = new PartitionCopy();
	int returnCode = pc->run(argc, argv);
	delete pc;
	return returnCode;
}
