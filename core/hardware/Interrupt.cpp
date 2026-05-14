#include "Interrupt.hpp"
#include "Logger.hpp"
#include "Scheduler.hpp"

Scheduler* Interrupt::scheduler = nullptr;

bool Interrupt::setStatus(bool enableInterrupts)
{
    sigset_t mask, old_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    if (enableInterrupts)
        sigprocmask(SIG_UNBLOCK, &mask, &old_mask);
    else
        sigprocmask(SIG_BLOCK, &mask, &old_mask);

    // sigismember returns 1 if SIGALRM is in the old_mask (meaning it was BLOCKED).
    // Therefore, if it equals 0, interrupts were ENABLED.
    return sigismember(&old_mask, SIGALRM) == 0;
}

void Interrupt::enable()
{
    Interrupt::setStatus(true);
}

bool Interrupt::disable()
{
    return Interrupt::setStatus(false);
}

void Interrupt::restore(bool status)
{
    Interrupt::setStatus(status);
}

void Interrupt::timerInterruptHandler(int)
{
    if (Interrupt::scheduler->getIsIdling()) return;
    LOG(INTERRUPT, INFO, "Timer interrupt fired.");
    Interrupt::scheduler->preempt();
}

void Interrupt::init(Scheduler* sched)
{
    Interrupt::scheduler = sched;
}