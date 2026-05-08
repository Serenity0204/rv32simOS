#pragma once

#include "Alarm.hpp"
#include "HAL.hpp"
#include "Loader.hpp"
#include "PageReplacementPolicy.hpp"
#include "ProcessManager.hpp"
#include "Scheduler.hpp"
#include "SwapManager.hpp"
#include "SyscallHandler.hpp"
#include "VFSInterface.hpp"
#include "VirtualMemoryManager.hpp"

struct Kernel
{
    // kernel core components
    Scheduler* scheduler = nullptr;
    ProcessManager* procManager = nullptr;
    SyscallHandler* syscalls = nullptr;
    VirtualMemoryManager* vmm = nullptr;
    Alarm* alarm = nullptr;
    SwapManager* swap = nullptr;

    // filesystem related
    PageReplacementPolicy* pageReplacementPolicy = nullptr;
    VFSInterface* vfs = nullptr;

    // hardware abstraction
    HAL* hal = nullptr;

    Kernel() = default;
    ~Kernel() = default;
};

namespace kernelfunction
{
    void initKernelSubsystem(Kernel* kernel);
    void destroyKernelSubsystem(Kernel* kernel);
};