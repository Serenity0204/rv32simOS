#include "Process.hpp"
#include "KernelInstance.hpp"
#include "Logger.hpp"

Process::Process(int id, std::string name)
{
    this->pcb = new PCB(id, name);
}

Process::~Process()
{
    if (this->pcb != nullptr)
    {
        delete this->pcb;
        this->pcb = nullptr;
    }
}

Thread* Process::createThread(Addr entryPC, Word arg)
{
    if (this->pcb->threads.size() >= MAX_THREADS) return nullptr;

    bool isExecutable = false;

    // check the passed in address is valid
    for (const Segment& seg : this->pcb->segments)
    {
        if (entryPC >= seg.vaddr && entryPC < (seg.vaddr + seg.memSize))
        {
            if (seg.flags & PF_X)
            {
                isExecutable = true;
                break;
            }
        }
    }
    if (!isExecutable) return nullptr;

    Addr stackSize = this->pcb->threads.empty() ? MAIN_STACK_SIZE : THREAD_STACK_SIZE;
    Addr stackTop = this->pcb->nextStackBase;

    // for next thread
    this->pcb->nextStackBase = stackTop - stackSize - GUARD_PAGE_SIZE;

    int tid = static_cast<int>(this->pcb->threads.size());
    Thread* thread = new Thread(tid, this, entryPC, stackTop);

    thread->getRegs().write(10, arg);
    this->pcb->threads.push_back(thread);
    return thread;
}

int Process::createMutex()
{
    Mutex* mutex = new Mutex();
    int id = this->pcb->mutexList.size();
    this->pcb->mutexList.push_back(mutex);
    return id;
}

Mutex* Process::getMutex(int id)
{
    if (id < 0 || static_cast<std::size_t>(id) >= this->pcb->mutexList.size()) return nullptr;
    return this->pcb->mutexList[id];
}

int Process::addFileHandle(FileHandleInterface* handle)
{
    if (handle == nullptr) return -1;

    // find the first ever fd slot
    int fd = -1;
    for (std::size_t i = 0; i < this->pcb->fdTable.size(); ++i)
    {
        if (this->pcb->fdTable[i] == nullptr)
        {
            fd = static_cast<int>(i);
            break;
        }
    }

    // no space
    if (fd == -1) return -1;
    this->pcb->fdTable[fd] = handle;
    return fd;
}

FileHandleInterface* Process::getFileHandle(int fd)
{
    if (fd < 0 || fd >= static_cast<int>(this->pcb->fdTable.size())) return nullptr;
    return this->pcb->fdTable[fd];
}

bool Process::closeFileHandle(int fd)
{
    if (fd < 0 || fd >= static_cast<int>(this->pcb->fdTable.size())) return false;
    FileHandleInterface* h = this->pcb->fdTable[fd];
    if (h == nullptr) return false;
    h->close();
    delete h;
    this->pcb->fdTable[fd] = nullptr;
    return true;
}

Addr Process::sbrk(int increment)
{
    Addr oldBreak = this->pcb->programBreak;
    Addr newBreak = this->pcb->programBreak + increment;

    if (newBreak < HEAP_START || newBreak > HEAP_MAX_LIMIT) return 0;
    this->pcb->programBreak = newBreak;
    return oldBreak;
}

void Process::wipeMemory()
{
    for (auto const& [vpn, pte] : *this->pcb->pageTable)
    {
        if (pte.valid)
        {
            Addr paddr = pte.ppn * KERNEL_PAGE_SIZE;
            kernel.systemCtx->pmm.freeFrame(paddr);
        }
    }
}

void Process::recycle(std::string newName)
{
    // give back physical mem
    this->wipeMemory();
    this->pcb->instructionsExecuted = 0;

    // reset page table
    this->pcb->pageTable->clear();

    // clear threads but do not delete them
    this->pcb->threads.clear();

    // clear old segments and mutexes
    this->pcb->segments.clear();
    for (Mutex* m : this->pcb->mutexList) delete m;
    this->pcb->mutexList.clear();

    // reset FDs
    for (size_t i = 3; i < this->pcb->fdTable.size(); ++i)
    {
        if (this->pcb->fdTable[i])
        {
            this->pcb->fdTable[i]->close();
            delete this->pcb->fdTable[i];
            this->pcb->fdTable[i] = nullptr;
        }
    }

    // reset VM pointers and proc name
    this->pcb->programBreak = HEAP_START;
    this->pcb->nextStackBase = STACK_TOP;
    this->pcb->name = newName;

    // set the process to be active
    this->pcb->active = true;
}

bool Process::terminate(int pid, int exitCode, bool crashed)
{
    if (pid < 0 || static_cast<size_t>(pid) >= kernel.systemCtx->processList.size())
    {
        LOG(KERNEL, ERROR, "Killing process with PID: " + std::to_string(pid) + " that does not exist.");
        return false;
    }

    Process* process = kernel.systemCtx->processList[pid];
    if (!process->isActive())
    {
        LOG(KERNEL, ERROR, "Killing process with PID: " + std::to_string(pid) + " that is not active.");
        return false;
    }

    // free physical memory
    process->wipeMemory();

    // terminate all threads
    for (Thread* thread : process->getThreads())
        thread->setState(ThreadState::TERMINATED);

    // cache exit code for any joiners
    kernel.systemCtx->exitCodes[pid] = exitCode;

    // wake up any threads blocked waiting for this process
    if (kernel.systemCtx->processWaiters.count(pid))
    {
        for (Thread* waiter : kernel.systemCtx->processWaiters[pid])
            waiter->setState(ThreadState::READY);
        kernel.systemCtx->processWaiters.erase(pid);
    }

    // record the stats
    STATS.recordProcessStats(pid, process->getName(), process->getTotalInstructions());

    // return slot to pool
    process->setActive(false);

    if (crashed)
    {
        LOG(KERNEL, INFO, "Killing Process " + std::to_string(pid) + " (CRASHED)");
        return true;
    }
    LOG(KERNEL, INFO, "Process " + std::to_string(pid) + " (Group Exit) terminated with code " + std::to_string(exitCode));

    return true;
}