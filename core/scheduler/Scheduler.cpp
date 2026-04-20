#include "Scheduler.hpp"
#include "Interrupt.hpp"
#include "KernelInstance.hpp"
#include "Logger.hpp"
#include "Thread.hpp"

void Scheduler::preempt()
{
    bool prev = Interrupt::disable();

    kernel.timerCtx->software.tick();
    if (kernel.systemCtx->activeThreads.empty())
    {
        Interrupt::restore(prev);
        return;
    }

    int prevIndex = kernel.systemCtx->currentThreadIndex;
    int nextIndex = prevIndex;
    std::size_t attempts = 0;
    bool found = false;
    std::size_t count = kernel.systemCtx->activeThreads.size();

    do
    {
        nextIndex = (nextIndex + 1) % count;
        attempts++;
        if (kernel.systemCtx->activeThreads[nextIndex]->getState() == ThreadState::READY)
        {
            found = true;
            break;
        }
    } while (attempts < count);

    if (!found)
    {
        bool canRunCurrentProcess = this->checkCurrentThreadRunnable();
        if (canRunCurrentProcess)
        {
            Interrupt::restore(prev);
            return;
        }
        // Check if everyone is terminated
        bool allDead = this->checkAllTerminated();
        if (allDead)
        {
            LOG(SCHEDULER, INFO, "All threads terminated.");
            void* dummy_sp = nullptr;
            context_switch(&dummy_sp, kernel.systemCtx->mainStackPointer);
            return;
        }
        LOG(SCHEDULER, INFO, "System IDLE: All threads blocked. Waiting for interrupts...");
        this->isIdling = true;
        while (!found)
        {
            sigset_t empty_mask;
            sigemptyset(&empty_mask);
            sigsuspend(&empty_mask);

            // tick the soft timer
            kernel.timerCtx->software.tick();

            // Check if the software timer wakes up a thread
            for (std::size_t i = 0; i < count; ++i)
            {
                nextIndex = (nextIndex + 1) % count;
                if (kernel.systemCtx->activeThreads[nextIndex]->getState() == ThreadState::READY)
                {
                    found = true;
                    break;
                }
            }
        }
        this->isIdling = false;
    }

    if (nextIndex != prevIndex)
    {
        Thread* nextThread = kernel.systemCtx->activeThreads[nextIndex];
        Process* proc = nextThread->getProcess();
        LOG(SCHEDULER, INFO, "Switching to Thread " + std::to_string(nextThread->getTid()) + " (PID " + std::to_string(proc->getPid()) + ")");
        this->contextSwitch(nextIndex);
    }
    Interrupt::restore(prev);
}

void Scheduler::contextSwitch(std::size_t nextIndex)
{
    STATS.incContextSwitches();

    int prevIndex = kernel.systemCtx->currentThreadIndex;
    Thread* prevThread = (prevIndex != -1) ? kernel.systemCtx->activeThreads[prevIndex] : nullptr;
    Thread* nextThread = kernel.systemCtx->activeThreads[nextIndex];

    // Layer 1: Swap RISCV State
    if (prevThread != nullptr && prevThread->getState() != ThreadState::TERMINATED)
    {
        prevThread->getRegs() = kernel.systemCtx->cpu.getRegs();
        prevThread->setPC(kernel.systemCtx->cpu.getPC());

        if (prevThread->getState() == ThreadState::RUNNING) prevThread->setState(ThreadState::READY);
    }

    kernel.systemCtx->cpu.getRegs() = nextThread->getRegs();
    kernel.systemCtx->cpu.setPC(nextThread->getPC());
    nextThread->setState(ThreadState::RUNNING);

    // check if it's switch within the same process
    if (prevThread == nullptr || prevThread->getProcess()->getPid() != nextThread->getProcess()->getPid())
        kernel.systemCtx->cpu.setPageTable(nextThread->getProcess()->getPageTable());

    kernel.systemCtx->currentThreadIndex = nextIndex;

    // Layer 2: Swap Host C++ State
    if (prevThread != nullptr)
    {
        context_switch(&prevThread->hostStackPointer, nextThread->hostStackPointer);
        return;
    }

    // First Boot! Jump away from the main loop context
    context_switch(&kernel.systemCtx->mainStackPointer, nextThread->hostStackPointer);
}

bool Scheduler::checkCurrentThreadRunnable()
{
    Thread* current = kernel.systemCtx->getCurrentThread();
    if (current->getState() == ThreadState::RUNNING) return true;
    return false;
}

bool Scheduler::checkAllTerminated()
{
    for (auto* t : kernel.systemCtx->activeThreads)
    {
        if (t->getState() != ThreadState::TERMINATED) return false;
    }
    return true;
}

void Scheduler::sleepCurrentThread(int delayMs, const std::string& reason)
{
    Thread* current = kernel.systemCtx->getCurrentThread();
    if (current->getState() == ThreadState::BLOCKED)
    {
        // the thread is already blocked Natively extend its underlying timer.
        bool extended = kernel.timerCtx->software.extendTimer(current->sleepTimerId, delayMs);
        if (extended)
            LOG(SCHEDULER, INFO, "Extended sleep for Thread " + std::to_string(current->getTid()) + " by " + std::to_string(delayMs) + "ms for " + reason);
        return;
    }

    current->setState(ThreadState::BLOCKED);
    LOG(SCHEDULER, INFO, "Thread " + std::to_string(current->getTid()) + " BLOCKED for " + reason + " (" + std::to_string(delayMs) + "ms)");

    current->sleepTimerId = kernel.timerCtx->software.registerTimer(delayMs, [current, reason]()
    {
        current->setState(ThreadState::READY);
        current->sleepTimerId = 0;
        LOG(SCHEDULER, INFO, reason + " Complete: Waking up Thread " + std::to_string(current->getTid()));
    });
}