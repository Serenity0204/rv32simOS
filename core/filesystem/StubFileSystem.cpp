#include "StubFileSystem.hpp"
#include "Common.hpp"
#include "DiskFileHandle.hpp"
#include "KernelInstance.hpp"
#include <cmath>

StubFileSystem::StubFileSystem(DiskInterface* disk) : disk(disk), freeBlockPointer(0)
{
}

FileHandleInterface* StubFileSystem::openImpl(const std::string& filename)
{
    if (!this->fileTable.count(filename)) return nullptr;

    FileMetadata meta = this->fileTable.at(filename);
    return new DiskFileHandle(this->disk, meta.startBlock, meta.sizeBytes);
}

bool StubFileSystem::createFileImpl(const std::string& filename, std::size_t sizeBytes)
{
    if (this->fileTable.count(filename) > 0) return false;

    // Ceiling division: (size + 4095) / 4096
    std::size_t blocksNeeded = (sizeBytes + BLOCK_SIZE - 1) / BLOCK_SIZE;

    // bump style disk block allocator
    if (this->freeBlockPointer + blocksNeeded > this->disk->numBlocks()) return false;

    // allocate meta data
    FileMetadata meta;
    meta.name = filename;
    meta.startBlock = this->freeBlockPointer;
    meta.sizeBytes = sizeBytes;

    this->fileTable[filename] = meta;

    // bump the ptr
    this->freeBlockPointer += blocksNeeded;
    return true;
}

bool StubFileSystem::removeFileImpl(const std::string& filename)
{
    if (!this->fileTable.count(filename)) return false;
    this->fileTable.erase(filename);
    return true;
}