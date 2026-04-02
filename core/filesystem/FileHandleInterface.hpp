#pragma once
#include "Common.hpp"
#include <cstddef>
#include <vector>

class FileHandleInterface
{
public:
    enum class Type
    {
        Console = 0,
        DiskFile = 1,
    };

public:
    virtual ~FileHandleInterface() = default;

    // Returns number of bytes read/written
    virtual int read(std::vector<Byte>& buffer, std::size_t size) = 0;
    virtual int write(const std::vector<Byte>& buffer, std::size_t size) = 0;
    virtual void close() {}
    virtual FileHandleInterface::Type type() = 0;
};