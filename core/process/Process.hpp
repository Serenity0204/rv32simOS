#pragma once
#include "Common.hpp"
#include "ConsoleHandle.hpp"
#include "FileHandleInterface.hpp"
#include "Mutex.hpp"
#include "PTE.hpp"
#include "RegFile.hpp"
#include "Segment.hpp"
#include "Thread.hpp"
#include <iostream>
#include <string>
#include <vector>

class Process
{
private:
    struct PCB
    {
        // info
        int pid;
        std::string name;
        bool active;

        // shared info
        PageTable* pageTable;
        std::vector<Segment> segments;
        std::vector<Thread*> threads;
        std::vector<Mutex*> mutexList;
        std::vector<FileHandleInterface*> fdTable;
        Addr programBreak;
        Addr nextStackBase;

        // for stats
        std::size_t instructionsExecuted;

        PCB(int id, std::string n) : pid(id), name(n)
        {
            this->instructionsExecuted = 0;
            this->pageTable = new PageTable();
            this->nextStackBase = STACK_TOP;
            this->programBreak = HEAP_START;
            this->active = false;

            this->fdTable.resize(FD_TABLE_SIZE);
            this->fdTable[0] = new ConsoleHandle(FileDescriptor::STDIN);
            this->fdTable[1] = new ConsoleHandle(FileDescriptor::STDOUT);
            this->fdTable[2] = new ConsoleHandle(FileDescriptor::STDERR);
            for (std::size_t i = 3; i < FD_TABLE_SIZE; ++i) this->fdTable[i] = nullptr;
        }

        ~PCB()
        {
            if (this->pageTable != nullptr) delete this->pageTable;
            this->threads.clear();

            for (Mutex* mutex : this->mutexList) delete mutex;

            for (FileHandleInterface* handle : this->fdTable)
            {
                if (handle != nullptr)
                    delete handle;
            }

            this->fdTable.clear();
            this->mutexList.clear();
        }
    };

public:
    Process(int id, std::string name);
    ~Process();

    Thread* createThread(Addr entryPC, Word arg);

    // core memory and lifecycle management
    void wipeMemory();
    void recycle(std::string newName);

public:
    static bool terminate(int pid, int exitCode, bool crashed);

public:
    // getters and setters
    inline int getPid() const { return this->pcb->pid; }
    inline std::string getName() const { return this->pcb->name; }
    inline void setName(std::string n) { this->pcb->name = n; }

    inline PageTable* getPageTable() { return this->pcb->pageTable; }
    inline std::vector<Segment>& getSegments() { return this->pcb->segments; }
    inline std::vector<Thread*>& getThreads() { return this->pcb->threads; }

    inline void setActive(bool val) { this->pcb->active = val; }
    inline bool isActive() const { return this->pcb->active; }

public:
    // for stats
    inline void incrementInstruction() { this->pcb->instructionsExecuted++; }
    inline std::size_t getTotalInstructions() const { return this->pcb->instructionsExecuted; }

public:
    // mutex related
    int createMutex();
    Mutex* getMutex(int id);

public:
    // file related
    int addFileHandle(FileHandleInterface* handle);
    FileHandleInterface* getFileHandle(int fd);
    bool closeFileHandle(int fd);

public:
    // heap related
    Addr sbrk(int increment);
    inline Addr getProgramBreak() const { return this->pcb->programBreak; }

private:
    PCB* pcb;
};