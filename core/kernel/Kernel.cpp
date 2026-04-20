#include "Kernel.hpp"
#include "Exception.hpp"
#include "Interrupt.hpp"
#include "KernelInstance.hpp"
#include "KernelPanic.hpp"
#include "Logger.hpp"
#include "ScopedCriticalSection.hpp"
#include "Stats.hpp"
#include "Utils.hpp"

Kernel::Kernel()
    : systemCtx(nullptr),
      storageCtx(nullptr),
      timerCtx(nullptr),
      scheduler(nullptr),
      vmm(nullptr),
      syscalls(nullptr),
      loader(nullptr)
{
    this->initInternal();
}

void Kernel::initInternal()
{
    this->systemCtx = new SystemContext();
    this->storageCtx = new StorageContext();
    this->storageCtx->init<StubFileSystem, InMemoryDisk, FIFOPolicy>(NUM_DISK_BLOCKS, NUM_SWAP_BLOCKS, this->systemCtx->pmm.getTotalFrames());
    this->timerCtx = new TimerContext();
    this->scheduler = new Scheduler();
    this->syscalls = new SyscallHandler();
    this->vmm = new VirtualMemoryManager();
    this->loader = new Loader();
}
void Kernel::destroyInternal()
{
    delete this->storageCtx;
    delete this->systemCtx;
    delete this->timerCtx;
    delete this->scheduler;
    delete this->vmm;
    delete this->syscalls;
    delete this->loader;
    Interrupt::init(nullptr);
}

Kernel::~Kernel()
{
    this->destroyInternal();
}

bool Kernel::createProcess(const std::string& filename)
{
    return this->loader->loadELF(filename) != -1;
}

bool Kernel::killProcess(int pid)
{
    return Process::terminate(pid, -1, true);
}

void Kernel::reset()
{
    this->destroyInternal();
    this->initInternal();
}

void Kernel::init()
{
    bool hasReady = !this->systemCtx->activeThreads.empty();
    if (!hasReady)
    {
        LOG(KERNEL, WARNING, "No READY processes.");
        return;
    }

    this->systemCtx->cpu.enableVM(true);

    Interrupt::enable();
    Interrupt::init(this->scheduler);
    Interrupt::disable();

    this->timerCtx->hardware.start(TIMER_INTERRUPT_FREQUENCY);
    LOG(KERNEL, INFO, "rv32umos Booting...");
    this->scheduler->preempt();

    this->timerCtx->hardware.stop();
}

void Kernel::runThread()
{
    Interrupt::enable();

    while (true)
    {
        // atomic check
        bool prev = Interrupt::disable();

        Thread* self = kernel.systemCtx->getCurrentThread();
        self->getProcess()->incrementInstruction();
        if (self->getState() == ThreadState::TERMINATED)
        {
            Interrupt::restore(prev);
            kernel.scheduler->preempt();
            return;
        }

        bool threadDead = false;

        try
        {
            STATS.incInstructions();
            kernel.systemCtx->cpu.step();
            kernel.systemCtx->cpu.advancePC();
        }
        catch (SyscallException& sys)
        {
            bool prev = Interrupt::disable();
            SyscallStatus status = kernel.syscalls->dispatch(sys.getSyscallID());

            if (status.error)
            {
                bool killed = kernel.killProcess(kernel.systemCtx->getCurrentThread()->getProcess()->getPid());
                if (!killed)
                {
                    PANIC("Failed to kill process after Syscall Error!");
                    return;
                }
                threadDead = true;
            }
            if (!status.error) Interrupt::restore(prev);
            if (!status.error && status.needReschedule) kernel.scheduler->preempt();
        }
        catch (PageFaultException& pf)
        {
            bool prev = Interrupt::disable();

            bool handled = kernel.vmm->handlePageFault(pf.getFaultAddr());
            if (!handled)
            {
                bool killed = kernel.killProcess(kernel.systemCtx->getCurrentThread()->getProcess()->getPid());
                if (!killed)
                {
                    PANIC("KERNEL PANIC: Failed to kill process after Segfault!");
                    return;
                }
                threadDead = true;
            }
            if (handled) Interrupt::restore(prev);
            if (handled && kernel.systemCtx->getCurrentThread()->getState() == ThreadState::BLOCKED) kernel.scheduler->preempt();
        }
        catch (std::exception& e)
        {
            Interrupt::disable();
            LOG(KERNEL, ERROR, "Unhandled C++ Exception: " + std::string(e.what()));

            bool killed = kernel.killProcess(kernel.systemCtx->getCurrentThread()->getProcess()->getPid());
            if (!killed)
            {
                PANIC("Failed to kill process after Exception!");
                return;
            }
            threadDead = true;
        }

        if (threadDead)
        {
            kernel.scheduler->preempt();
            // Thread is dead, returning is safe here
            return;
        }
    }

    PANIC("Unexpected error");
}
