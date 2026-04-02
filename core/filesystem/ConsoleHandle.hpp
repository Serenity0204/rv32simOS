#pragma once
#include "Common.hpp"
#include "FileHandleInterface.hpp"
#include <vector>

class ConsoleHandle : public FileHandleInterface
{
public:
    ConsoleHandle(int fd);
    ~ConsoleHandle() = default;

    int read(std::vector<Byte>& buffer, std::size_t size) override;
    int write(const std::vector<Byte>& buffer, std::size_t size) override;
    FileHandleInterface::Type type() override { return FileHandleInterface::Type::Console; }

private:
    int hostFD;
};