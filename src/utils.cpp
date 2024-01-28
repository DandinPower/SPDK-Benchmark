
#include <utils.h>
#include <sys/mman.h>
#include <memory.h>

void* memory_allocation(size_t size) {

#if MEM_ALLOCATION_MODE == SPDK_MODE
    return spdk_mem_allocation(size);
#elif MEM_ALLOCATION_MODE == POSIX_ALIGN_MODE
    void* buffer;
    posix_memalign((void **)&buffer, ALIGN_SIZE, size);
    return buffer;
#elif MEM_ALLOCATION_MODE == HUGE_PAGE_MODE
    void *m;
    size_t s = 8UL * 1024 * 1024;

    m = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);

    if (m == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }
    return m;
#else 
    return malloc(size);
#endif
}

void memory_free(void* buffer, size_t size) {
#if MEM_ALLOCATION_MODE == SPDK_MODE
    spdk_mem_free(buffer);
#elif MEM_ALLOCATION_MODE == HUGE_PAGE_MODE
    munmap(buffer, size);
#else 
    free(buffer);
#endif
}