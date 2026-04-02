#pragma once
#include "Common.hpp"
#include "SyscallDefinition.hpp"
#include <exception>
#include <string>

class SyscallException : public std::exception
{
public:
    SyscallException(std::string msg, SyscallID syscallID) : message(std::move(msg)), syscallID(syscallID) {}
    const char* what() const noexcept override { return this->message.c_str(); }
    SyscallID getSyscallID() { return this->syscallID; }

private:
    std::string message;
    SyscallID syscallID;
};

class PageFaultException : public std::exception
{
public:
    PageFaultException(Addr addr) : faultAddr(addr), message(std::string("Page Fault at 0x" + std::to_string(addr))) {}
    const char* what() const noexcept override { return this->message.c_str(); }
    Addr getFaultAddr() const { return this->faultAddr; }

private:
    Addr faultAddr;
    std::string message;
};