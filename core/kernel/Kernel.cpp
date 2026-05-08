#include "Kernel.hpp"
#include "Interrupt.hpp"
#include "KernelService.hpp"

void kernelfunction::initKernelSubsystem(Kernel* kernel)
{
    // init HAL
    kernel->hal = new HAL();
    KernelService::registerService("hal", kernel->hal);

    // init kernel subsystem
    kernel->procManager = new ProcessManager();
    kernel->scheduler = new Scheduler();
    kernel->syscalls = new SyscallHandler();
    kernel->vmm = new VirtualMemoryManager();
    kernel->alarm = new Alarm();
    KernelService::registerService("procManager", kernel->procManager);
    KernelService::registerService("scheduler", kernel->scheduler);
    KernelService::registerService("syscalls", kernel->syscalls);
    KernelService::registerService("vmm", kernel->vmm);
    KernelService::registerService("alarm", kernel->alarm);

    // init filesystem and swap
    kernel->pageReplacementPolicy = new PageReplacementPolicyImpl();
    kernel->pageReplacementPolicy->init(kernel->hal->pmm.getTotalFrames());
    kernel->vfs = new VFSImpl(kernel->hal->disk);
    kernel->swap = new SwapManager(kernel->hal->disk, NUM_DISK_BLOCKS, NUM_SWAP_BLOCKS);

    KernelService::registerService("prp", kernel->pageReplacementPolicy);
    KernelService::registerService("vfs", kernel->vfs);
    KernelService::registerService("swap", kernel->swap);
}

void kernelfunction::destroyKernelSubsystem(Kernel* kernel)
{
    delete kernel->swap;
    delete kernel->vfs;
    delete kernel->pageReplacementPolicy;

    delete kernel->vmm;
    delete kernel->syscalls;
    delete kernel->scheduler;
    delete kernel->procManager;
    delete kernel->alarm;

    delete kernel->hal;
    KernelService::clear();
    Interrupt::init(nullptr);
}
