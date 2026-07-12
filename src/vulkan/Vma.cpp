// VMA 需要此宏，必须在包含 vk_mem_alloc.h 之前定义。
// 并且只能在一个单独的 .cpp 文件中定义。
#define VMA_IMPLEMENTATION

#define VMA_STATIC_VULKAN_FUNCTIONS 0

#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#include "vk_mem_alloc.h"
