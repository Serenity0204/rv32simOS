#pragma once
#include "Common.hpp"
#include "DiskInterface.hpp"
#include "FileHandleInterface.hpp"
#include <vector>

class DiskFileHandle : public FileHandleInterface
{
public:
    DiskFileHandle(DiskInterface* disk, std::size_t startBlock, std::size_t sizeBytes);
    ~DiskFileHandle() = default;

    int read(std::vector<Byte>& buffer, std::size_t size) override;
    int write(const std::vector<Byte>& buffer, std::size_t size) override;
    FileHandleInterface::Type type() override { return FileHandleInterface::Type::DiskFile; }

    int seek(int offset);
    int reset();
    bool eof();

private:
    DiskInterface* disk;
    std::size_t startBlockIndex;
    std::size_t fileSize;
    Addr cursor;
};