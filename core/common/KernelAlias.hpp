#pragma once
#include "Kernel.hpp"
#include "KernelService.hpp"

// hardware abstraction layer
#define K_HAL KernelService::get<HAL>("hal")

// core OS components
#define K_PROC_MANAGER KernelService::get<ProcessManager>("procManager")
#define K_SCHEDULER KernelService::get<Scheduler>("scheduler")
#define K_SYSCALLS KernelService::get<SyscallHandler>("syscalls")
#define K_VMM KernelService::get<VirtualMemoryManager>("vmm")
#define K_ALARM KernelService::get<Alarm>("alarm")
#define K_SWAP KernelService::get<SwapManager>("swap")

// filesystem related
#define K_PRP KernelService::get<PageReplacementPolicy>("prp")
#define K_VFS KernelService::get<VFSInterface>("vfs")

// kernel itself
#define K_KERNEL KernelService::get<Kernel>("kernel")