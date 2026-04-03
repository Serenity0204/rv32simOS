# rv32umos
> **RISCV-32 User Mode Operating System (rv32umos)** is an OS kernel running on a custom RISC-V architectural simulator inspired by [Nachos](https://en.wikipedia.org/wiki/Not_Another_Completely_Heuristic_Operating_System)

## Overview
**rv32umos** 
is an OS kernel that runs on a custom 32-bit RISC-V architectural simulator using **High-Level Emulation (HLE)** written in C++. It features virtual memory with demand paging and swapping, preemptive multi-thread scheduling(driven by POSIX timer), synchronization primitives, a virtual file system, and a custom syscall ABI. It serves an educational purpose by bridging the gap between hardware emulation and OS theory.

## Architecture
The system employs a **Sequential Execution Model with Signal-Driven Interrupt Polling**.
- **User Space**: Runs unmodified RISC-V ELF binaries. Instructions are fetched, decoded, and executed by the emulated CPU.
- **Kernel Space (Host)**: The OS logic (Scheduler, VMM, VFS, Syscalls) runs as native C++ code.
- **Preemption & POSIX Timer**: 
    - **Context Switching**:  The simulator utilizes the **POSIX timer** to trigger hardware interrupts at a configurable frequency. When a timer signal is detected, it goes into the **Timer Interrupt Handler** and trigger context switching via host level `swapcontext` and kernel RISCV context switching.
    - **Interrupt Handling**: To ensure kernel-mode atomicity and prevent race conditions within the simulator, interrupt can be turned on and off to ensure that critical sections are protected.

## Project Structure
The project follows a flat, modular directory structure:
* `common/`: Generic C++ utilities, logging, and statistics.
* `disk/`: Physical/Simulated disk hardware interfaces and memory-disks.
* `filesystem/`: VFS, File Handles, and High-level logic (StubFS).
* `hardware/`: RISC-V Silicon simulation (CPU, MMU, Bus, Decoder, Executor).
* `interrupt/`: Global interrupt management and POSIX signal handling.
* `kernel/`: OS lifecycle, context management, and main kernel loop.
* `memory/`: Paging, Swapping, and Physical/Virtual memory management (PTE, Segments).
* `process/`: ELF loading and Process Control Blocks (PCB).
* `scheduler/`: Round-Robin logic and Software Timers.
* `sync/`: Concurrency primitives (Mutexes, Scoped Locks).
* `syscall/`: The User-to-Kernel API boundary and dispatching.
* `thread/`: Execution contexts and Thread Control Blocks (TCB).

## Features
- **Emulated Hardware**
  - **RV32IM ISA**: Full support for base integer instructions + M-extension (Multiplication/Division).
  - **Cost Modeling**: Simulates hardware latency by charging "Time Costs" for instructions and I/O events.
- **Preemptive Multitasking**
  - **POSIX Driven**: Real-time preemption using host signals to simulate hardware timer ticks.
  - **Multi-Threading**: Native support for kernel-managed threads. Processes can spawn multiple threads that share the same page table.
  - **Synchronization**: Provides syncrhonization mechanism including Lock, Semaphore, Condition Variable, and Barrier.
  - **Scheduler**: Implements Round-Robin scheduling with a configurable time quantum. Context switches save and restore full hardware context (registers, PC).
- **Memory Management**
  - **Virtual Memory**: Per-process isolation using page tables.
  - **Lazy Loading**: Code and data segments are demand-paged directly from the ELF executable, and stack memory grows dynamically.
  - **Protection**: Enforces Read/Write/Execute permissions defined in the ELF headers.
  - **Swap & Eviction**: Dedicated Swap Manager for memory overcommitment with configurable eviction policies.
  - **Userspace Memory Allocation(Malloc/Free)**: Implemented the heap memory allocator for userspace by utilizing the custom sbrk syscall.
- **Storage & File System**
  - **Virtual File System (VFS)**: An abstract layer routing file operations to their respective handlers
  - **In-Memory Disk**: Simulates a block device with 4KB sectors.
  - **Stub File System**: A lightweight contiguous-allocation filesystem that tracks file metadata and disk blocks.
  - **File Handles**: Polymorphic file descriptors allowing uniform read/write calls across the Console and Disk.

## System Calls
  - `SYS_WRITE`: Handles output to stdout/stderr.
  - `SYS_READ`: Basic input handling from stdin.
  - `SYS_EXIT`: Manages process termination and exit codes.
  - `SYS_THREAD_CREATE`: Creates a thread object.
  - `SYS_THREAD_EXIT`: Exit the current thread.
  - `SYS_THREAD_JOIN`: Join one thread with another thread.
  - `SYS_MUTEX_CREATE`: Create a mutex lock for this process.
  - `SYS_MUTEX_LOCK`: Try to acquire the lock.
  - `SYS_MUTEX_UNLOCK`: Try to release the lock.
  - `SYS_OPEN`: open a file, return the file descriptor id.
  - `SYS_CLOSE`: close the file.
  - `SYS_CREATE`: create a file.
  - `SYS_SBRK`: expand or shrink the heap
  - `SYS_CREATE_PROCESS`: ELF-based process creation (spawn).
  - `SYS_JOIN_PROCESS`: Block parent and wait for child termination.

  
## [Detailed Wiki](https://deepwiki.com/Serenity0204/rv32umos)

## Toolchain Requirements
1. Install the riscv toolchain
  ```bash
  gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf
  ```
2. CMake
3. g++

## Usage

**Compile project and C code**
```bash
bash build.sh
```
**Clean**
```bash
bash clean.sh
```
**Run**
- Without Logs
```bash
./rv32umos programs/<your_file.elf>
```
- With Logs
```bash
./rv32umos programs/<your_file.elf> -logs
```

## Example Output(Log)
```
Hello From Meow!
───────────────────────────────────────
───▐▀▄───────▄▀▌───▄▄▄▄▄▄▄─────────────
───▌▒▒▀▄▄▄▄▄▀▒▒▐▄▀▀▒██▒██▒▀▀▄──────────
──▐▒▒▒▒▀▒▀▒▀▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▀▄────────
──▌▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▄▒▒▒▒▒▒▒▒▒▒▒▒▀▄──────
▀█▒▒▒█▌▒▒█▒▒▐█▒▒▒▀▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▌─────
▀▌▒▒▒▒▒▒▀▒▀▒▒▒▒▒▒▀▀▒▒▒▒▒▒▒▒▒▒▒▒▒▒▐───▄▄
▐▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▌▄█▒█
▐▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▒█▀─
▐▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒█▀───
▐▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▌────
─▌▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▐─────
─▐▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▌─────
──▌▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▐──────
──▐▄▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▄▌──────
────▀▄▄▀▀▀▀▀▄▄▀▀▀▀▀▀▀▄▄▀▀▀▀▀▄▄▀────────

Logs:=======================================
[LOADER   ] Segment: 0x0 Size: 3917 Flags: 5
[LOADER   ] Segment: 0x0 Size: 0 Flags: 6
[LOADER   ] Created Process 0: programs/test3.elf
[KERNEL   ] Simulation started...
[SCHEDULER] Switching to Thread 0 (PID 0)
[MMU      ] Segment Page Loaded: 0x0
[MMU      ] Thread 0 BLOCKED for Lazy Load (10ms)
[SCHEDULER] System IDLE: All threads blocked. Waiting for interrupts...
[MMU      ] Lazy Load Complete: Waking up Thread 0
[MMU      ] Stack Page Allocated: 0x3fffffc
[SYSCALL  ] Write called by PID 0
[SYSCALL  ] Write called by PID 0
[SYSCALL  ] Write called by PID 0
[INTERRUPT] Timer interrupt fired.
[INTERRUPT] Timer interrupt fired.
[INTERRUPT] Timer interrupt fired.
[INTERRUPT] Timer interrupt fired.
[INTERRUPT] Timer interrupt fired.
[SYSCALL  ] Write called by PID 0
[KERNEL   ] Process 0 (Group Exit) terminated with code 67
[SCHEDULER] All threads terminated.

============================================
            MACHINE STATISTICS              
============================================
Total Instructions       :      15593
Total Syscalls           :          5
Total Page Faults        :          2
Total Page Swap Ins      :          0
Total Page Swap Outs     :          0
Context Switches         :          1
Disk Reads (4KB)         :          1
Disk Writes (4KB)        :          0
Physical Frames Used     :          2
--------------------------------------------
 Paging Rate: 0.0128% (Faults/Instr)
============================================

```


## Todo
- [X] Multi-Threading
- [ ] Synchronization
- [ ] IPC
- [X] Swap and Page Eviction
- [X] Virtual Disk
- [X] More Syscalls
- [X] Filesystem
- [X] Configurable Scheduling/Swap Policies
- [ ] Module Loading

