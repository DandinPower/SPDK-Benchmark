#pragma once
// constants
#define MB 1024 * 1024
#define NVME_SECTOR_SIZE 512

// configurations
#define TEST_ROUNDS 100
#define BUFFER_SIZE 134217728
#define CHUNK_SIZE 33554432 / 4
#define ALIGN_SIZE NVME_SECTOR_SIZE

// MEM related configurations
#define MEM_CPY_MODE 0 // MEM_COPY_MODE: 0 only work for MEM_ALLOCATION_MODE = SPDK_MODE
#define SPDK_MODE 1
#define POSIX_ALIGN_MODE 2
#define HUGE_PAGE_MODE 3
#define MEM_ALLOCATION_MODE SPDK_MODE 