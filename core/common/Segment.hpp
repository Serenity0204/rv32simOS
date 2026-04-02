#pragma once
#include "Common.hpp"

// for ELF
struct Segment
{
    Addr vaddr;
    size_t memSize;
    size_t fileSize;
    size_t fileOffset;
    uint32_t flags;
};