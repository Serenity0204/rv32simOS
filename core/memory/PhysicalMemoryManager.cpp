#include "PhysicalMemoryManager.hpp"
#include "KernelInstance.hpp"
#include "KernelPanic.hpp"

void PhysicalMemoryManager::init()
{
    for (Addr frame = MEMORY_BASE; frame < MEMORY_BASE + MEMORY_SIZE; frame += KERNEL_PAGE_SIZE)
        this->freeFrames.push_back(frame);

    std::size_t totalFrames = MEMORY_SIZE / KERNEL_PAGE_SIZE;
    this->frameTable.resize(totalFrames);
}

FrameAllocInfo PhysicalMemoryManager::allocateFrame()
{
    kernel.scheduler->sleepCurrentThread(MEMORY_ALLOCATION_TIME, "Memory Allocation");
    // status = false to trigger swap
    FrameAllocInfo info;
    if (this->freeFrames.empty())
    {
        info.paddr = 0;
        info.status = false;
        return info;
    }

    Addr frame = this->freeFrames.back();
    this->freeFrames.pop_back();
    info.paddr = frame;
    info.status = true;
    return info;
}

void PhysicalMemoryManager::freeFrame(Addr frame)
{
    this->freeFrames.push_back(frame);
    Addr ppn = frame >> 12;
    if (ppn >= this->frameTable.size()) PANIC("PPN Out of Bound!");
    this->frameTable[ppn].clear();
}

void PhysicalMemoryManager::registerFrameOwner(Addr ppn, Addr vpn, int ownerPid)
{
    if (ppn >= this->frameTable.size()) PANIC("PPN Out of Bound!");
    this->frameTable[ppn].allocated = true;
    this->frameTable[ppn].ownerPid = ownerPid;
    this->frameTable[ppn].vpn = vpn;
}
