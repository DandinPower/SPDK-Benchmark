#pragma once
#include "spdk/env.h"
#include <config.h>

// Based on different configurations, we can use different memory allocation
// void* mem_allocation(size_t size);