#pragma once
#include "DiskInterface.hpp"
#include "HardwareTimer.hpp"
#include "InMemoryDisk.hpp"
#include "KernelPanic.hpp"
#include "Machine.hpp"
#include "PageReplacementPolicy.hpp"
#include "PhysicalMemoryManager.hpp"
#include "Process.hpp"
#include "SoftwareTimer.hpp"
#include "Stats.hpp"
#include "StubFileSystem.hpp"
#include "SwapManager.hpp"
#include "VFSInterface.hpp"
#include <unordered_map>
#include <vector>

class Kernel;

struct SystemContext
{
public:
    Machine cpu;
    PhysicalMemoryManager pmm;

    // Maps PID -> Exit Code
    std::unordered_map<int, int> exitCodes;
    // Maps PID -> List of Threads waiting for that PID to exit
    std::unordered_map<int, std::vector<Thread*>> processWaiters;

    std::vector<Process*> processList;
    std::vector<Thread*> activeThreads;
    int currentThreadIndex = -1;

    // host stuff
    void* mainStackPointer = nullptr;

    SystemContext()
    {
        this->pmm.init();
        for (int i = 0; i < MAX_PROCESS; ++i)
        {
            Process* p = new Process(i, "");
            p->setActive(false);
            this->processList.push_back(p);
        }
    }

    ~SystemContext()
    {
        for (auto* p : this->processList) delete p;
        this->processList.clear();
        for (auto* t : this->activeThreads) delete t;
        this->activeThreads.clear();
    }

    inline Thread* getCurrentThread()
    {
        if (currentThreadIndex == -1) PANIC("Current thread index is -1");
        if (static_cast<std::size_t>(currentThreadIndex) >= activeThreads.size()) PANIC("Current thread index out of bound");
        return this->activeThreads[this->currentThreadIndex];
    }
};

struct StorageContext
{
    VFSInterface* vfs = nullptr;
    DiskInterface* disk = nullptr;
    PageReplacementPolicy* pageReplacementPolicy = nullptr;
    SwapManager* swap = nullptr;

    template <typename VFSImpl, typename DiskImpl, typename PageReplacementPolicyImpl>
    void init(std::size_t numDiskBlocks, std::size_t numSwapBlocks, std::size_t totalFrames)
    {
        static_assert(std::is_base_of<VFSInterface, VFSImpl>::value,
                      "VFSImpl must derive from VFSInterface");
        static_assert(std::is_base_of<DiskInterface, DiskImpl>::value,
                      "DiskImpl must derive from DiskInterface");
        static_assert(std::is_base_of<PageReplacementPolicy, PageReplacementPolicyImpl>::value,
                      "PageReplacementPolicyImpl must derive from PageReplacementPolicy");

        this->disk = new DiskImpl(numDiskBlocks);
        this->vfs = new VFSImpl(this->disk);
        this->pageReplacementPolicy = new PageReplacementPolicyImpl();
        this->pageReplacementPolicy->init(totalFrames);
        this->swap = new SwapManager(this->disk, numDiskBlocks, numSwapBlocks);
    }

    ~StorageContext()
    {
        delete this->swap;
        delete this->pageReplacementPolicy;
        delete this->vfs;
        delete this->disk;
    }
};

struct TimerContext
{
public:
    HardwareTimer hardware;
    SoftwareTimer software;

    TimerContext() = default;
    ~TimerContext() = default;
};