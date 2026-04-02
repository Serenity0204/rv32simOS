#include "ConsoleHandle.hpp"
#include <iostream>
#include <unistd.h>

ConsoleHandle::ConsoleHandle(int fd) : hostFD(fd) {}

int ConsoleHandle::read(std::vector<Byte>& buffer, std::size_t size)
{
    if (this->hostFD == FileDescriptor::STDIN)
    {
#ifndef __EMSCRIPTEN__
        ssize_t bytesRead = ::read(FileDescriptor::STDIN, buffer.data(), size);
        return bytesRead;
#else
        // web case, unhandled yet
        return -1;
#endif
    }
    return -1;
}

int ConsoleHandle::write(const std::vector<Byte>& buffer, std::size_t size)
{
    if (this->hostFD == FileDescriptor::STDOUT || this->hostFD == FileDescriptor::STDERR)
    {
        ssize_t written = ::write(this->hostFD, buffer.data(), size);
        std::cout.flush();
        return written;
    }
    return -1;
}