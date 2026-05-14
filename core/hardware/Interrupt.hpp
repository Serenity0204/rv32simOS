#pragma once
#include <signal.h>

class Scheduler;

class Interrupt
{

private:
    Interrupt() = default;
    ~Interrupt() = default;
    Interrupt(const Interrupt&) = delete;
    Interrupt& operator=(const Interrupt&) = delete;
    static bool setStatus(bool enableInterrupts);

private:
    static Scheduler* scheduler;

public:
    static void enable();
    static bool disable();
    static void restore(bool status);

    static void init(Scheduler* sched);
    static void timerInterruptHandler(int signum);
};
