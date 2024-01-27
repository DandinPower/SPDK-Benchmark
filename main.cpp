// #include <spdk_engine.h>
#include <phison_engine.h>
#include <config.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include <utils.h>

size_t get_aligned_size(size_t alignment, size_t size) {
    assert((alignment & (alignment - 1)) == 0);  // Ensure alignment is a power of 2
    return (size + alignment - 1) & ~(alignment - 1);
}

void check_error(bool condition, const std::string &msg) {
    if (condition) {
        std::cerr << msg << ": " << std::strerror(errno) << std::endl;
        std::exit(1);
    }
}

void generate_random_data(char* buffer, size_t buffer_size) {
    for (size_t i = 0; i < buffer_size; ++i) {
        buffer[i] = rand() % 256;
    }
}

int
main(int argc, char **argv)
{
	double totalWriteLatency = 0.0;
    double totalReadLatency = 0.0;

	size_t bufferSize = BUFFER_SIZE;
	size_t bufferSizeAligned = get_aligned_size(NVME_SECTOR_SIZE , bufferSize);

    // Allocate aligned memory for write and read
    // char *writeBuffer, *readBuffer;
    // check_error(posix_memalign((void **)&writeBuffer, NVME_SECTOR_SIZE, bufferSizeAligned), "posix_memalign error for write_data");
    // check_error(posix_memalign((void **)&readBuffer, NVME_SECTOR_SIZE, bufferSizeAligned), "posix_memalign error for read_data");
	
	char *writeBuffer = (char*) mem_allocation(bufferSizeAligned);
	char *readBuffer = (char*) mem_allocation(bufferSizeAligned);
	
	// char *writeBuffer = (char*) mem_allocation(BUFFER_SIZE);
	generate_random_data(writeBuffer, bufferSizeAligned);

	// memset(writeBuffer, 0, BUFFER_SIZE);
	// char *readBuffer = (char*) mem_allocation(bufferSizeAligned);

	auto startWrite = std::chrono::high_resolution_clock::now();
	int ret = processor(writeBuffer, 1, 0, bufferSizeAligned);
	auto endWrite = std::chrono::high_resolution_clock::now();

	auto startRead = std::chrono::high_resolution_clock::now();
	ret = processor(readBuffer, 0, 0, bufferSizeAligned);
	auto endRead = std::chrono::high_resolution_clock::now();

	for (int i=0; i < TEST_ROUNDS; i++) {

		auto startWrite = std::chrono::high_resolution_clock::now();
		int ret = processor(writeBuffer, 1, 0, bufferSizeAligned);
		auto endWrite = std::chrono::high_resolution_clock::now();

		auto startRead = std::chrono::high_resolution_clock::now();
		ret = processor(readBuffer, 0, 0, bufferSizeAligned);
		auto endRead = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> writeTime = endWrite - startWrite;
		std::chrono::duration<double> readTime = endRead - startRead;

		totalWriteLatency += writeTime.count();
		totalReadLatency += readTime.count();

		// Compare write and read data
		if (memcmp(writeBuffer, readBuffer, bufferSizeAligned) == 0) {
			// std::cout << "Data is the same" << std::endl;
		} else {
			std::cout << "Data is different" << std::endl;
		}
	}

	size_t writeSpeedMBs = std::round(static_cast<double>(bufferSize) * TEST_ROUNDS / totalWriteLatency / static_cast<double>(MB));
	size_t readSpeedMBs = std::round(static_cast<double>(bufferSize) * TEST_ROUNDS / totalReadLatency / static_cast<double>(MB));
	std::cout << "Avg Write speed: " << writeSpeedMBs << " MB/s" << std::endl;
	std::cout << "Avg Read speed: " << readSpeedMBs << " MB/s" << std::endl;

	// Free allocated memory
	// free(writeBuffer);
	// free(readBuffer);
	mem_free(writeBuffer);
	mem_free(readBuffer);
	return 0;
}