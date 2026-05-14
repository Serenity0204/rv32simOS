#include "MMU.hpp"
#include "Exception.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

MMU::MMU()
{
    this->reset();
}

void MMU::setMemory(Memory* memory)
{
    this->memory = memory;
}

void MMU::reset()
{
    this->vmEnabled = false;
    this->currentTable = nullptr;
}

void MMU::setPageTable(PageTable* table)
{
    this->currentTable = table;
}

void MMU::enableVM(bool enabled)
{
    this->vmEnabled = enabled;
}

Word MMU::loadVirtualMemory(Addr vaddr, std::size_t size)
{
    Addr paddr = this->translate(vaddr, false, false);
    return this->memory->load(paddr, size);
}

void MMU::storeVirtualMemory(Addr vaddr, std::size_t size, Word value)
{
    Addr paddr = this->translate(vaddr, true, false);
    this->memory->store(paddr, size, value);
}

Word MMU::loadPhysicalMemory(Addr paddr, std::size_t size)
{
    return this->memory->load(paddr, size);
}

void MMU::storePhysicalMemory(Addr paddr, std::size_t size, Word value)
{
    this->memory->store(paddr, size, value);
}

Word MMU::fetch(Addr vaddr)
{
    // Fetch is always 4 bytes
    Addr paddr = this->translate(vaddr, false, true);
    return this->memory->load(paddr, 4);
}

Addr MMU::translate(Addr vaddr, bool isWrite, bool isExec)
{
    if (!this->vmEnabled) return vaddr;

    uint32_t vpn = (vaddr >> 12);
    uint32_t offset = (vaddr & 0xFFF);

    if (this->currentTable == nullptr || this->currentTable->count(vpn) == 0) throw PageFaultException(vaddr);
    PTE& pte = this->currentTable->at(vpn);

    if (!pte.valid) throw PageFaultException(vaddr);

    if (isExec && !pte.canExecute)
    {
        LOG(MMU, ERROR, "Execution Violation at " + Utils::toHex(vaddr));
        throw PageFaultException(vaddr);
    }
    if (isWrite && !pte.canWrite)
    {
        LOG(MMU, ERROR, "Write Violation at " + Utils::toHex(vaddr));
        throw PageFaultException(vaddr);
    }
    if (!isWrite && !isExec && !pte.canRead)
    {
        LOG(MMU, ERROR, "Read Violation at " + Utils::toHex(vaddr));
        throw PageFaultException(vaddr);
    }

    pte.referenced = true;
    if (isWrite) pte.dirty = true;

    return (pte.ppn << 12) | offset;
}