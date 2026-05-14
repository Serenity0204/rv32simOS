#pragma once
#include "Common.hpp"
#include "Memory.hpp"
#include "PTE.hpp"

class MMU
{
public:
    MMU();
    ~MMU() = default;

    // virtual memory
    Word loadVirtualMemory(Addr vaddr, std::size_t size);
    void storeVirtualMemory(Addr vaddr, std::size_t size, Word value);

    // physical memory direct access
    Word loadPhysicalMemory(Addr paddr, std::size_t size);
    void storePhysicalMemory(Addr paddr, std::size_t size, Word value);

    // fetch instruction
    Word fetch(Addr vaddr);

    // Control
    void setPageTable(PageTable* table);
    void enableVM(bool enabled);
    void reset();
    void setMemory(Memory* memory);

private:
    Memory* memory = nullptr;
    PageTable* currentTable;
    bool vmEnabled;

    Addr translate(Addr vaddr, bool isWrite, bool isExec);
};