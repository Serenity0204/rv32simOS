#pragma once

#include "Common.hpp"
#include "MMU.hpp"
#include "RegFile.hpp"

class Bus;
class Executor;

class Machine
{
public:
    friend class Executor;
    Machine();
    ~Machine() = default;

    void step();
    void reset();
    void dumpRegisters();

    // PC
    inline Addr getPC() { return this->pc; }
    inline void advancePC() { this->pc += 4; }
    inline void setPC(Addr newPc) { this->pc = newPc; }

    // memory
    inline Word loadVirtualMemory(Addr vaddr, std::size_t size) { return this->mmu.loadVirtualMemory(vaddr, size); }
    inline void storeVirtualMemory(Addr vaddr, std::size_t size, Word value) { this->mmu.storeVirtualMemory(vaddr, size, value); }
    inline Word loadPhysicalMemory(Addr paddr, std::size_t size) { return this->mmu.loadPhysicalMemory(paddr, size); }
    inline void storePhysicalMemory(Addr paddr, std::size_t size, Word value) { this->mmu.storePhysicalMemory(paddr, size, value); }

    // reg
    inline Word readReg(std::size_t index) { return this->regs[index]; }
    inline void writeReg(std::size_t index, Word value) { this->regs.write(index, value); }
    inline RegFile& getRegs() { return this->regs; }

    // VM Control
    inline void setPageTable(PageTable* table) { this->mmu.setPageTable(table); }
    inline void enableVM(bool enabled) { this->mmu.enableVM(enabled); }

private:
    // kernel related
    RegFile regs;
    MMU mmu;
    Addr pc;

    Word fetch();
    void execute(Word instr);
};
