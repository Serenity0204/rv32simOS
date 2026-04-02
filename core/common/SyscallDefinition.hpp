#pragma once
#include <string>
#include <unordered_map>

struct SyscallStatus
{
    bool needReschedule = false;
    bool error = false;
};

enum class SyscallID : int
{
    SYS_EXIT = 0,
    SYS_WRITE = 1,
    SYS_READ = 2,
    SYS_THREAD_CREATE = 3,
    SYS_THREAD_EXIT = 4,
    SYS_THREAD_JOIN = 5,
    SYS_MUTEX_CREATE = 6,
    SYS_MUTEX_LOCK = 7,
    SYS_MUTEX_UNLOCK = 8,
    SYS_OPEN = 9,
    SYS_CLOSE = 10,
    SYS_CREATE = 11,
    SYS_SBRK = 12,
    SYS_CREATE_PROCESS = 13,
    SYS_JOIN_PROCESS = 14,
    SYS_UNKNOWN = -1,
};

inline const std::unordered_map<SyscallID, std::string> sysCallNameMap = {
    {SyscallID::SYS_EXIT, "exit syscall"},
    {SyscallID::SYS_WRITE, "write syscall"},
    {SyscallID::SYS_READ, "read syscall"},
    {SyscallID::SYS_THREAD_CREATE, "create thread syscall"},
    {SyscallID::SYS_THREAD_EXIT, "exit thread syscall"},
    {SyscallID::SYS_THREAD_JOIN, "join thread syscall"},
    {SyscallID::SYS_MUTEX_CREATE, "mutex create syscall"},
    {SyscallID::SYS_MUTEX_LOCK, "mutex lock syscall"},
    {SyscallID::SYS_MUTEX_UNLOCK, "mutex unlock syscall"},
    {SyscallID::SYS_OPEN, "open yscall"},
    {SyscallID::SYS_CLOSE, "close syscall"},
    {SyscallID::SYS_CREATE, "create syscall"},
    {SyscallID::SYS_SBRK, "sbrk syscall"},
    {SyscallID::SYS_CREATE_PROCESS, "create process syscall"},
    {SyscallID::SYS_JOIN_PROCESS, "join process syscall"},
};
