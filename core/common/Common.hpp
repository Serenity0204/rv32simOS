#pragma once
#include <cstdint>
#include <iostream>

// riscv32
using Byte = uint8_t;
using Half = uint16_t;
using Word = uint32_t;
using Addr = uint32_t;

// Max process
const int MAX_PROCESS = 8;
const int MAX_THREADS = 5;

// 1kb
const Addr KERNEL_PAGE_SIZE = 4096;

// Safety gap between stacks (4096)
const Addr GUARD_PAGE_SIZE = KERNEL_PAGE_SIZE;

// Currently 64 KB
const std::size_t PHYSICAL_RAM_KB = 64;
const Addr MEMORY_SIZE = PHYSICAL_RAM_KB * 1024;
const Addr MEMORY_BASE = 0x00000000;

// Currently 64 MB Virtual Space
const std::size_t VIRTUAL_RAM_MB = 64;
const Addr VIRTUAL_MEMORY_SIZE = VIRTUAL_RAM_MB * 1024 * 1024;

// ============================================================================
// STACK LAYOUT
// ============================================================================
// Stack 32 mb, grows top to down
const Addr STACK_TOP = VIRTUAL_MEMORY_SIZE;
const Addr STACK_REGION_BOTTOM = VIRTUAL_MEMORY_SIZE / 2;

// Main Thread gets fixed 25%, what child gets depending on the num of threads
const Addr MAIN_STACK_SIZE = VIRTUAL_MEMORY_SIZE / 4;
const Addr THREAD_STACK_SIZE = ((VIRTUAL_MEMORY_SIZE / 4) / (MAX_THREADS - 1)) - GUARD_PAGE_SIZE;

// ============================================================================
// HEAP LAYOUT
// ============================================================================
// First 16mb is elf, heap start at 16mb, grows upward till 32mb
const Addr HEAP_START = VIRTUAL_MEMORY_SIZE / 4;
const Addr HEAP_MAX_LIMIT = STACK_REGION_BOTTOM;

// for context switch
const int TIMER_INTERRUPT_FREQUENCY = 1;

// constants for timer modeling
const std::size_t USER_MODE_TICK_TIME = 1;
const std::size_t ENTER_KERNEL_MODE_TIME = 20;
const std::size_t CONTEXT_SWITCH_TIME = 50;
const std::size_t SYSCALL_BASE_TIME = 10;
const std::size_t MEMORY_ALLOCATION_TIME = 100;
const std::size_t DISK_IO_TIME = 5000;

// for disk
const std::size_t BLOCK_SIZE = 4096;
const std::size_t NUM_DISK_BLOCKS = 4096;
const std::size_t NUM_SWAP_BLOCKS = 2048;

// fd table size
const std::size_t FD_TABLE_SIZE = 1024;

// max file name length
const std::size_t MAX_FILE_NAME_LENGTH = 128;

// FD
enum FileDescriptor : int
{
    STDIN = 0,
    STDOUT = 1,
    STDERR = 2
};
