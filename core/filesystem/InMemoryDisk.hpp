#pragma once
#include "Common.hpp"
#include "DiskInterface.hpp"
#include <vector>

class InMemoryDisk : public DiskInterface
{
public:
    InMemoryDisk(std::size_t numBlocks);
    ~InMemoryDisk() = default;

    void readBlock(std::size_t blockIndex, std::vector<Byte>& buffer) override;
    void writeBlock(std::size_t blockIndex, const std::vector<Byte>& buffer) override;
    std::size_t numBlocks() const override;

private:
    std::vector<Byte> data;
    std::size_t totalBlocks;
};