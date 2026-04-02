#pragma once
#include "Common.hpp"
#include <elf.h>
#include <unordered_map>

struct PTE
{
public:
    Addr ppn = 0;
    bool valid = false;
    bool referenced = false;
    bool dirty = false;
    bool swapped = false;

    // permission bits
    bool canRead = false;
    bool canWrite = false;
    bool canExecute = false;

public:
    static void setPTEFlagsFromElf(PTE& pte, uint32_t flags)
    {
        pte.canRead = (flags & PF_R);
        pte.canWrite = (flags & PF_W);
        pte.canExecute = (flags & PF_X);
    }
};

using PageTable = std::unordered_map<Addr, PTE>;