#pragma once
#include "DiskInterface.hpp"
#include "HardwareTimer.hpp"
#include "Machine.hpp"
#include "Memory.hpp"
#include "SystemConfig.hpp"

struct HAL
{
public:
    Memory physicalRAM;
    Machine cpu;
    HardwareTimer timer;
    DiskInterface* disk = nullptr;

    HAL()
    {
        this->cpu.setMemory(&this->physicalRAM);
        this->disk = new DiskImpl(NUM_DISK_BLOCKS);
    }

    ~HAL()
    {
        delete this->disk;
    }
};
