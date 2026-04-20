#include "SyscallHandler.hpp"
#include "Common.hpp"
#include "Kernel.hpp"
#include "KernelInstance.hpp"
#include "KernelPanic.hpp"
#include "Logger.hpp"
#include "Stats.hpp"
#include "Utils.hpp"
#include <iostream>
#include <unistd.h>
#include <vector>

SyscallStatus SyscallHandler::dispatch(SyscallID id)
{
    STATS.incSyscalls();

    SyscallStatus status;
    switch (id)
    {
    case SyscallID::SYS_EXIT:
        handleExit(status);
        break;
    case SyscallID::SYS_WRITE:
        this->handleWrite(status);
        break;
    case SyscallID::SYS_READ:
        this->handleRead(status);
        break;
    case SyscallID::SYS_THREAD_CREATE:
        this->handleThreadCreate(status);
        break;
    case SyscallID::SYS_THREAD_EXIT:
        this->handleThreadExit(status);
        break;
    case SyscallID::SYS_THREAD_JOIN:
        this->handleThreadJoin(status);
        break;
    case SyscallID::SYS_MUTEX_CREATE:
        this->handleMutexCreate(status);
        break;
    case SyscallID::SYS_MUTEX_LOCK:
        this->handleMutexLock(status);
        break;
    case SyscallID::SYS_MUTEX_UNLOCK:
        this->handleMutexUnlock(status);
        break;
    case SyscallID::SYS_OPEN:
        this->handleOpen(status);
        break;
    case SyscallID::SYS_CLOSE:
        this->handleClose(status);
        break;
    case SyscallID::SYS_CREATE:
        this->handleCreate(status);
        break;
    case SyscallID::SYS_SBRK:
        this->handleSbrk(status);
        break;
    case SyscallID::SYS_CREATE_PROCESS:
        this->handleCreateProcess(status);
        break;
    case SyscallID::SYS_JOIN_PROCESS:
        this->handleJoinProcess(status);
        break;
    default:
        PANIC("Unimplemented syscall id: " + std::to_string((int)id));
        break;
    }

    return status;
}

void SyscallHandler::handleThreadJoin(SyscallStatus& status)
{
    // reset status
    status.needReschedule = false;
    status.error = false;

    int targetThreadTid = kernel.systemCtx->cpu.readReg(10);
    Thread* current = kernel.systemCtx->getCurrentThread();
    Process* process = current->getProcess();

    Thread* targetThread = nullptr;

    // see if the target exist in the first place
    std::vector<Thread*>& threads = process->getThreads();
    for (Thread* thread : threads)
    {
        if (thread->getTid() == targetThreadTid)
        {
            targetThread = thread;
            break;
        }
    }

    // no need to reschedule, if the target does not exist in the first place
    if (targetThread == nullptr)
    {
        LOG(SYSCALL, ERROR, "Thread Join Failed: Target Thread ID does not exist " + std::to_string(targetThreadTid));
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }

    // cannot join itself
    if (targetThread->getTid() == current->getTid())
    {
        LOG(SYSCALL, ERROR, "Thread Join Failed: Cannot join with yourself");
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }

    // has been joined
    if (targetThread->getHasBeenJoined())
    {
        LOG(SYSCALL, ERROR, "Thread Join Failed: Target Thread with ID " + std::to_string(targetThreadTid) + " has been joined by other threads");
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }

    // update status
    targetThread->setHasBeenJoined(true);

    // target already dead, return success and continue
    if (targetThread->getState() == ThreadState::TERMINATED)
    {
        kernel.systemCtx->cpu.writeReg(10, targetThread->getExitCode());
        kernel.systemCtx->cpu.advancePC();
        return;
    }

    // target still running, block the current thread
    LOG(SYSCALL, INFO, "Thread " + std::to_string(current->getTid()) + " blocking for Thread " + std::to_string(targetThreadTid));

    // set the id and block the current thread
    targetThread->setJoiner(current);
    current->setState(ThreadState::BLOCKED);
    status.needReschedule = true;
}

void SyscallHandler::handleThreadExit(SyscallStatus& status)
{
    // reset status
    status.needReschedule = false;
    status.error = false;

    Word exitCode = kernel.systemCtx->cpu.readReg(10);
    Thread* current = kernel.systemCtx->getCurrentThread();

    current->setState(ThreadState::TERMINATED);
    current->setExitCode(exitCode);
    Process* proc = current->getProcess();
    LOG(KERNEL, INFO, "Thread " + std::to_string(current->getTid()) + " (PID " + std::to_string(proc->getPid()) + ")" + " exited code " + std::to_string(exitCode));

    // check for joiner
    Thread* joiner = current->getJoiner();
    if (joiner != nullptr)
    {
        joiner->setState(ThreadState::READY);
        // pass the exit code to joiner for their ecall
        joiner->getRegs().write(10, exitCode);
        // advance joiner PC
        joiner->setPC(joiner->getPC() + 4);
        LOG(SYSCALL, INFO, "Thread " + std::to_string(current->getTid()) + " waking up Joiner " + std::to_string(joiner->getTid()));
    }
    // Signal to schedule
    status.needReschedule = true;
    return;
}

void SyscallHandler::handleThreadCreate(SyscallStatus& status)
{
    // reset status
    status.needReschedule = false;
    status.error = false;

    Word funcPtr = kernel.systemCtx->cpu.readReg(10);
    Word arg = kernel.systemCtx->cpu.readReg(11);

    Thread* current = kernel.systemCtx->getCurrentThread();

    Process* proc = current->getProcess();
    Thread* newThread = proc->createThread(funcPtr, arg);
    if (newThread == nullptr)
    {
        LOG(SYSCALL, ERROR, "Thread Creation Failed");
        // Return -1 to user
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }
    newThread->setupHostContext(reinterpret_cast<void (*)()>(&Kernel::runThread));
    newThread->setState(ThreadState::READY);
    kernel.systemCtx->activeThreads.push_back(newThread);

    LOG(SYSCALL, INFO, "Created Thread " + std::to_string(newThread->getTid()) + " (PID " + std::to_string(proc->getPid()) + ")");

    kernel.systemCtx->cpu.writeReg(10, newThread->getTid());
    kernel.systemCtx->cpu.advancePC();
}

void SyscallHandler::handleWrite(SyscallStatus& status)
{
    // reset status
    status.needReschedule = false;
    status.error = false;

    Thread* currentThread = kernel.systemCtx->getCurrentThread();

    Process* current = currentThread->getProcess();
    LOG(SYSCALL, DEBUG, "Write called by PID " + std::to_string(current->getPid()));

    Word rawFD = kernel.systemCtx->cpu.readReg(10);
    Word addr = kernel.systemCtx->cpu.readReg(11);
    Word count = kernel.systemCtx->cpu.readReg(12);

    // get file handle
    int fd = static_cast<int>(rawFD);
    FileHandleInterface* handle = current->getFileHandle(fd);
    // if not valid
    if (handle == nullptr)
    {
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }

    std::vector<Byte> buffer(count);
    for (Word i = 0; i < count; ++i)
    {
        char c = kernel.systemCtx->cpu.loadVirtualMemory(addr + static_cast<Addr>(i), 1);
        buffer[i] = c;
    }

    // will block if it's disk IO
    int written = handle->write(buffer, count);
    kernel.systemCtx->cpu.writeReg(10, written);
    kernel.systemCtx->cpu.advancePC();

    if (currentThread->getState() == ThreadState::BLOCKED) status.needReschedule = true;
}

void SyscallHandler::handleRead(SyscallStatus& status)
{
    // reset status
    status.needReschedule = false;
    status.error = false;

    Thread* currentThread = kernel.systemCtx->getCurrentThread();
    Process* current = currentThread->getProcess();

    LOG(SYSCALL, DEBUG, "Read called by PID " + std::to_string(current->getPid()));

    Word rawFD = kernel.systemCtx->cpu.readReg(10);
    Word addr = kernel.systemCtx->cpu.readReg(11);
    Word count = kernel.systemCtx->cpu.readReg(12);

    // get file handle
    int fd = static_cast<int>(rawFD);
    FileHandleInterface* handle = current->getFileHandle(fd);
    // if not valid
    if (handle == nullptr)
    {
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }

    std::vector<Byte> buffer(count);
    int bytesRead = handle->read(buffer, count);
    if (bytesRead > 0)
    {
        // Copy back to User VM
        for (int i = 0; i < bytesRead; ++i)
        {
            char rawChar = buffer[i];
            Byte byte = static_cast<uint8_t>(rawChar);
            kernel.systemCtx->cpu.storeVirtualMemory(addr + static_cast<Addr>(i), 1, static_cast<Word>(byte));
        }
    }
    kernel.systemCtx->cpu.writeReg(10, bytesRead);
    kernel.systemCtx->cpu.advancePC();

    if (currentThread->getState() == ThreadState::BLOCKED) status.needReschedule = true;
}

void SyscallHandler::handleOpen(SyscallStatus& status)
{
    status.needReschedule = false;
    status.error = false;

    Word pathAddr = kernel.systemCtx->cpu.readReg(10);
    Thread* currentThread = kernel.systemCtx->getCurrentThread();
    std::string filename;
    // read virtual memory string
    std::size_t offset = 0;
    while (true)
    {
        char c = static_cast<char>(kernel.systemCtx->cpu.loadVirtualMemory(pathAddr + offset, 1));
        if (c == 0) break;
        filename += c;
        ++offset;
        if (offset > MAX_FILE_NAME_LENGTH) break;
    }

    FileHandleInterface* handle = kernel.storageCtx->vfs->open(filename);
    if (handle == nullptr)
    {
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }

    // file exists
    Process* current = kernel.systemCtx->getCurrentThread()->getProcess();
    int fd = current->addFileHandle(handle);
    LOG(SYSCALL, INFO, "Opened file: " + filename + " (FD " + std::to_string(fd) + ")");

    if (fd == -1)
    {
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }

    kernel.systemCtx->cpu.writeReg(10, fd);
    kernel.systemCtx->cpu.advancePC();

    if (currentThread->getState() == ThreadState::BLOCKED) status.needReschedule = true;
}

void SyscallHandler::handleClose(SyscallStatus& status)
{
    status.needReschedule = false;
    status.error = false;
    Word fd = kernel.systemCtx->cpu.readReg(10);

    Process* current = kernel.systemCtx->getCurrentThread()->getProcess();
    bool ok = current->closeFileHandle(fd);
    int code = ok ? 0 : 1;
    kernel.systemCtx->cpu.writeReg(10, code);
    kernel.systemCtx->cpu.advancePC();
}

void SyscallHandler::handleCreate(SyscallStatus& status)
{
    status.needReschedule = false;
    status.error = false;

    Word pathAddr = kernel.systemCtx->cpu.readReg(10);
    Word size = kernel.systemCtx->cpu.readReg(11);
    Thread* currentThread = kernel.systemCtx->getCurrentThread();
    std::string filename;
    // read virtual memory string
    std::size_t offset = 0;
    while (true)
    {
        char c = static_cast<char>(kernel.systemCtx->cpu.loadVirtualMemory(pathAddr + offset, 1));
        if (c == 0) break;
        filename += c;
        ++offset;
        if (offset > MAX_FILE_NAME_LENGTH) break;
    }

    bool success = kernel.storageCtx->vfs->createFile(filename, size);
    if (!success)
    {
        LOG(SYSCALL, ERROR, "Failed to create file: " + filename);
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }

    LOG(SYSCALL, INFO, "Created file: " + filename + " (Size: " + std::to_string(size) + ")");
    kernel.systemCtx->cpu.writeReg(10, 0);
    kernel.systemCtx->cpu.advancePC();

    if (currentThread->getState() == ThreadState::BLOCKED) status.needReschedule = true;
}

void SyscallHandler::handleSbrk(SyscallStatus& status)
{
    // reset status
    status.needReschedule = false;
    status.error = false;

    int increment = static_cast<int>(kernel.systemCtx->cpu.readReg(10));
    Process* current = kernel.systemCtx->getCurrentThread()->getProcess();
    Addr oldBreak = current->sbrk(increment);

    // sbrk failed
    if (oldBreak == 0)
    {
        LOG(SYSCALL, ERROR, "Sbrk failed: Out of Heap Memory");
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }

    LOG(SYSCALL, DEBUG, "Sbrk successful. New Break: " + Utils::toHex(current->getProgramBreak()));
    kernel.systemCtx->cpu.writeReg(10, oldBreak);
    kernel.systemCtx->cpu.advancePC();
}

void SyscallHandler::handleMutexCreate(SyscallStatus& status)
{
    // reset status
    status.needReschedule = false;
    status.error = false;

    Thread* current = kernel.systemCtx->getCurrentThread();
    int mutexID = current->getProcess()->createMutex();
    kernel.systemCtx->cpu.writeReg(10, mutexID);
    kernel.systemCtx->cpu.advancePC();
}

void SyscallHandler::handleMutexLock(SyscallStatus& status)
{
    // reset status
    status.needReschedule = false;
    status.error = false;

    int mutexID = kernel.systemCtx->cpu.readReg(10);
    Thread* current = kernel.systemCtx->getCurrentThread();
    Process* process = current->getProcess();
    Mutex* lock = process->getMutex(mutexID);

    // not valid lock, error
    if (lock == nullptr)
    {
        status.error = true;
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }

    bool acquired = lock->acquire(current);

    // if lock is held, need to reschedule, and dont advance PC so later when all woke up, it can retry
    if (!acquired)
    {
        status.needReschedule = true;
        return;
    }

    // lock is acquired, just advance PC and return 0
    kernel.systemCtx->cpu.writeReg(10, 0);
    kernel.systemCtx->cpu.advancePC();
}
void SyscallHandler::handleMutexUnlock(SyscallStatus& status)
{
    // reset status
    status.needReschedule = false;
    status.error = false;

    int mutexID = kernel.systemCtx->cpu.readReg(10);
    Thread* current = kernel.systemCtx->getCurrentThread();
    Process* process = current->getProcess();
    Mutex* lock = process->getMutex(mutexID);

    // not valid lock
    if (lock == nullptr)
    {
        status.error = true;
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }

    // try to release
    bool success = lock->release(current);

    // not the owner releasing the lock
    if (!success)
    {
        status.error = true;
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }

    // releasing success
    kernel.systemCtx->cpu.writeReg(10, 0);
    kernel.systemCtx->cpu.advancePC();
}

void SyscallHandler::handleCreateProcess(SyscallStatus& status)
{
    status.needReschedule = false;
    status.error = false;

    // read virtual memory string for filename
    Word pathAddr = kernel.systemCtx->cpu.readReg(10);
    std::string filename;
    std::size_t offset = 0;
    while (true)
    {
        char c = static_cast<char>(kernel.systemCtx->cpu.loadVirtualMemory(pathAddr + offset, 1));
        if (c == 0) break;
        filename += c;
        if (++offset > MAX_FILE_NAME_LENGTH) break;
    }

    int createdPid = kernel.loader->loadELF(filename);
    if (createdPid == -1)
    {
        LOG(SYSCALL, ERROR, "Create process failed for file: " + filename);
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }
    kernel.systemCtx->cpu.writeReg(10, createdPid);
    kernel.systemCtx->cpu.advancePC();
}

void SyscallHandler::handleJoinProcess(SyscallStatus& status)
{
    status.needReschedule = false;
    status.error = false;

    int targetPid = static_cast<int>(kernel.systemCtx->cpu.readReg(10));
    Word statusPtr = kernel.systemCtx->cpu.readReg(11);

    // Fast Path: process already dead
    if (kernel.systemCtx->exitCodes.count(targetPid))
    {
        int exitCode = kernel.systemCtx->exitCodes[targetPid];
        if (statusPtr != 0)
            kernel.systemCtx->cpu.storeVirtualMemory(statusPtr, 4, static_cast<Word>(exitCode));

        kernel.systemCtx->exitCodes.erase(targetPid);
        kernel.systemCtx->cpu.writeReg(10, 1);
        kernel.systemCtx->cpu.advancePC();
        LOG(SYSCALL, INFO, "Join instantly reaped cached Exit Code for PID " + std::to_string(targetPid));
        return;
    }

    bool validPID = (targetPid >= 0) && (targetPid < MAX_PROCESS) && kernel.systemCtx->processList[targetPid]->isActive();

    // invalid PID
    if (!validPID)
    {
        LOG(SYSCALL, ERROR, "Join failed: invalid or inactive PID " + std::to_string(targetPid));
        kernel.systemCtx->cpu.writeReg(10, -1);
        kernel.systemCtx->cpu.advancePC();
        return;
    }

    // if process still running
    Thread* current = kernel.systemCtx->getCurrentThread();
    kernel.systemCtx->processWaiters[targetPid].push_back(current);
    current->setState(ThreadState::BLOCKED);
    status.needReschedule = true;
    LOG(SYSCALL, INFO, "Thread " + std::to_string(current->getTid()) + " BLOCKED waiting for PID " + std::to_string(targetPid));
    // PC is NOT advanced. When woken up, it will re-execute and hit Fast Path.
}

void SyscallHandler::handleExit(SyscallStatus& status)
{
    // reset status
    status.needReschedule = false;
    status.error = false;

    Word exitCode = kernel.systemCtx->cpu.readReg(10);
    Thread* current = kernel.systemCtx->getCurrentThread();

    Process* proc = current->getProcess();
    int currentPid = proc->getPid();

    bool ok = Process::terminate(currentPid, static_cast<int>(exitCode), false);
    if (!ok)
    {
        PANIC("KERNEL PANIC: Failed to terminate process " + std::to_string(currentPid));
        return;
    }
    status.needReschedule = true;
}