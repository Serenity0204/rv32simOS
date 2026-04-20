#pragma once
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

class Stats
{
private:
    struct ProcessStat
    {
        int pid;
        std::string name;
        std::size_t instr;
    };

public:
    static Stats& getInstance()
    {
        static Stats instance;
        return instance;
    }

    inline void incInstructions() { this->totalInstructions++; }
    inline void incContextSwitches() { this->totalContextSwitches++; }
    inline void incPageFaults() { this->totalPageFaults++; }
    inline void incSyscalls() { this->totalSyscalls++; }
    inline void incDiskReads() { this->totalDiskReads++; }
    inline void incDiskWrites() { this->totalDiskWrites++; }
    inline void incAllocatedFrames() { this->totalAllocatedFrames++; }
    inline void incSwapIns() { this->totalSwapIns++; }
    inline void incSwapOuts() { this->totalSwapOuts++; }

    // CPU usage per process
    inline void recordProcessStats(int pid, const std::string& name, std::size_t instr) { this->processStats.push_back({pid, name, instr}); }

    inline void printSummary()
    {
        std::cout << "\n============================================\n";
        std::cout << "            MACHINE STATISTICS              \n";
        std::cout << "============================================\n";

        printMetric("Total Instructions", this->totalInstructions);
        printMetric("Total Syscalls", this->totalSyscalls);
        printMetric("Total Page Faults", this->totalPageFaults);
        printMetric("Total Page Swap Ins", this->totalSwapIns);
        printMetric("Total Page Swap Outs", this->totalSwapOuts);
        printMetric("Context Switches", this->totalContextSwitches);
        printMetric("Disk Reads (4KB)", this->totalDiskReads);
        printMetric("Disk Writes (4KB)", this->totalDiskWrites);
        printMetric("Physical Frames Used", this->totalAllocatedFrames);

        std::cout << "--------------------------------------------\n";

        double pagingRate = (this->totalInstructions > 0) ? (double)this->totalPageFaults / this->totalInstructions * 100.0 : 0.0;

        std::cout << " Paging Rate: " << std::fixed << std::setprecision(4)
                  << pagingRate << "% (Faults/Instr)\n";

        std::cout << "============================================\n";
        this->generateProcessData();
    }

    inline void reset()
    {
        this->totalInstructions = 0;
        this->totalContextSwitches = 0;
        this->totalPageFaults = 0;
        this->totalSyscalls = 0;
        this->totalDiskReads = 0;
        this->totalDiskWrites = 0;
        this->totalAllocatedFrames = 0;
        this->totalSwapIns = 0;
        this->totalSwapOuts = 0;
    }

private:
    Stats() = default;

    uint64_t totalInstructions = 0;
    uint64_t totalContextSwitches = 0;
    uint64_t totalPageFaults = 0;
    uint64_t totalSyscalls = 0;
    uint64_t totalDiskReads = 0;
    uint64_t totalDiskWrites = 0;
    uint64_t totalAllocatedFrames = 0;
    uint64_t totalSwapIns = 0;
    uint64_t totalSwapOuts = 0;

    inline void printMetric(const std::string& label, uint64_t value)
    {
        std::cout << std::left << std::setw(25) << label
                  << ": " << std::right << std::setw(10) << value << "\n";
    }

    inline void generateProcessData()
    {
        std::ofstream outFile("process_stats.txt");

        for (ProcessStat& p : this->processStats)
        {
            int pid = p.pid;
            std::string name = p.name;
            std::size_t instr = p.instr;

            double percent = (this->totalInstructions > 0) ? ((double)instr / this->totalInstructions) * 100.0 : 0.0;

            outFile << pid << " " << name << " " << std::fixed << std::setprecision(2) << percent << "\n";
        }
        outFile.close();
    }
    std::vector<Stats::ProcessStat> processStats;
};

#define STATS Stats::getInstance()