#pragma once
#include "Common.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

class DiskInterface
{
public:
    virtual ~DiskInterface() = default;
    virtual void readBlock(std::size_t blockIndex, std::vector<Byte>& buffer) = 0;
    virtual void writeBlock(std::size_t blockIndex, const std::vector<Byte>& buffer) = 0;
    virtual std::size_t numBlocks() const = 0;
};