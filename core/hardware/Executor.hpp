#pragma once
#include "Common.hpp"

class Machine;

class Executor
{
public:
    Executor() = delete;
    ~Executor() = delete;
    Executor(const Executor&) = delete;
    Executor& operator=(const Executor&) = delete;

    // Immediates
    static void execLUI(Machine& cpu, Word instr);
    static void execAUIPC(Machine& cpu, Word instr);

    // I type
    static void execADDI(Machine& cpu, Word instr);
    static void execSLTI(Machine& cpu, Word instr);
    static void execSLTIU(Machine& cpu, Word instr);
    static void execXORI(Machine& cpu, Word instr);
    static void execORI(Machine& cpu, Word instr);
    static void execANDI(Machine& cpu, Word instr);
    static void execSLLI(Machine& cpu, Word instr);
    static void execSRLI(Machine& cpu, Word instr);
    static void execSRAI(Machine& cpu, Word instr);

    // R type
    static void execADD(Machine& cpu, Word instr);
    static void execSUB(Machine& cpu, Word instr);
    static void execSLL(Machine& cpu, Word instr);
    static void execSLT(Machine& cpu, Word instr);
    static void execSLTU(Machine& cpu, Word instr);
    static void execXOR(Machine& cpu, Word instr);
    static void execSRL(Machine& cpu, Word instr);
    static void execSRA(Machine& cpu, Word instr);
    static void execOR(Machine& cpu, Word instr);
    static void execAND(Machine& cpu, Word instr);

    // Others
    static void execFENCE(Machine& cpu, Word instr);
    static void execFENCEI(Machine& cpu, Word instr);

    // // CSR
    // static void execCSRRW(CPU& cpu, Word instr);
    // static void execCSRRS(CPU& cpu, Word instr);
    // static void execCSRRC(CPU& cpu, Word instr);
    // static void execCSRRWI(CPU& cpu, Word instr);
    // static void execCSRRSI(CPU& cpu, Word instr);
    // static void execCSRRCI(CPU& cpu, Word instr);

    // // System
    static void execECALL(Machine& cpu);
    // static void execEBREAK(CPU& cpu, Word instr);
    // static void execSRET(CPU& cpu, Word instr);
    // static void execURET(CPU& cpu, Word instr);
    // static void execMRET(CPU& cpu, Word instr);
    // static void execWFI(CPU& cpu, Word instr);
    // static void execSFENCEVMA(CPU& cpu, Word instr);

    // Load
    static void execLB(Machine& cpu, Word instr);
    static void execLH(Machine& cpu, Word instr);
    static void execLW(Machine& cpu, Word instr);
    static void execLBU(Machine& cpu, Word instr);
    static void execLHU(Machine& cpu, Word instr);

    // Store
    static void execSB(Machine& cpu, Word instr);
    static void execSH(Machine& cpu, Word instr);
    static void execSW(Machine& cpu, Word instr);

    // Jump
    static void execJAL(Machine& cpu, Word instr);
    static void execJALR(Machine& cpu, Word instr);

    // Branch
    static void execBEQ(Machine& cpu, Word instr);
    static void execBNE(Machine& cpu, Word instr);
    static void execBLT(Machine& cpu, Word instr);
    static void execBGE(Machine& cpu, Word instr);
    static void execBLTU(Machine& cpu, Word instr);
    static void execBGEU(Machine& cpu, Word instr);

    // M Extension
    static void execMUL(Machine& cpu, Word instr);
    static void execMULH(Machine& cpu, Word instr);
    static void execMULHSU(Machine& cpu, Word instr);
    static void execMULHU(Machine& cpu, Word instr);
    static void execDIV(Machine& cpu, Word instr);
    static void execDIVU(Machine& cpu, Word instr);
    static void execREM(Machine& cpu, Word instr);
    static void execREMU(Machine& cpu, Word instr);
};
