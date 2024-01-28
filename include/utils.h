#pragma once
#include <config.h>
#include <spdk_utils.h>

// Based on different configurations, we can use different memory allocation
void* memory_allocation(size_t size);

// Based on different configurations, we can use different memory free
void memory_free(void* buffer, size_t size);