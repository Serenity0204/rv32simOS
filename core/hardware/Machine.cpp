#include "Bus.hpp"
#include "Decoder.hpp"
#include "Exception.hpp"
#include "Executor.hpp"
#include "Machine.hpp"
#include "Utils.hpp"

Machine::Machine()
{
    this->reset();
}

void Machine::reset()
{
    this->mmu.reset();
    this->halted = false;
    this->pc = MEMORY_BASE;
    this->regs.reset();
    this->regs.write(2, STACK_TOP);
}

Word Machine::fetch()
{
    return this->mmu.fetch(this->pc);
}

void Machine::execute(Word instr)
{

    Opcode op = Decoder::opcode(instr);
    Func3 func3 = Decoder::func3(instr);
    Func7 func7 = Decoder::func7(instr);

    using namespace Utils;

    switch (op)
    {
    // R-Type Instructions (Register to Register)
    case Opcode::R_Type:
    {
        if (func7 == Func7::M_TYPE)
        {
            switch (func3)
            {
            case Func3::MUL:
                Executor::execMUL(*this, instr);
                break;
            case Func3::MULH:
                Executor::execMULH(*this, instr);
                break;
            case Func3::MULHSU:
                Executor::execMULHSU(*this, instr);
                break;
            case Func3::MULHU:
                Executor::execMULHU(*this, instr);
                break;
            case Func3::DIV:
                Executor::execDIV(*this, instr);
                break;
            case Func3::DIVU:
                Executor::execDIVU(*this, instr);
                break;
            case Func3::REM:
                Executor::execREM(*this, instr);
                break;
            case Func3::REMU:
                Executor::execREMU(*this, instr);
                break;
            default:
                illegal(instr, "unknown funct3 for <opcode>");
                break;
            }
            break;
        }

        switch (func3)
        {
        case Func3::ADD_SUB:
            switch (func7)
            {
            case Func7::ADD_SRL:
                Executor::execADD(*this, instr);
                break;
            case Func7::SUB_SRA:
                Executor::execSUB(*this, instr);
                break;
            default:
                illegal(instr, "ADD/SUB requires funct7=0 or 0b0100000");
                break;
            }
            break;

        case Func3::SLL:
            if (func7 != Func7::ADD_SRL) illegal(instr, "R-type SLL requires funct7=0");
            Executor::execSLL(*this, instr);
            break;

        case Func3::SLT:
            if (func7 != Func7::ADD_SRL) illegal(instr, "R-type SLT requires funct7=0");
            Executor::execSLT(*this, instr);
            break;

        case Func3::SLTU:
            if (func7 != Func7::ADD_SRL) illegal(instr, "R-type SLTU requires funct7=0");
            Executor::execSLTU(*this, instr);
            break;

        case Func3::XOR:
            if (func7 != Func7::ADD_SRL) illegal(instr, "R-type XOR requires funct7=0");
            Executor::execXOR(*this, instr);
            break;

        case Func3::SRL_SRA:
            switch (func7)
            {
            case Func7::ADD_SRL:
                Executor::execSRL(*this, instr);
                break;
            case Func7::SUB_SRA:
                Executor::execSRA(*this, instr);
                break;
            default:
                illegal(instr, "SRL/SRA requires funct7=0 or 0b0100000");
                break;
            }

            break;

        case Func3::OR:
            if (func7 != Func7::ADD_SRL) illegal(instr, "R-type OR requires funct7=0");
            Executor::execOR(*this, instr);
            break;

        case Func3::AND:
            if (func7 != Func7::ADD_SRL) illegal(instr, "R-type AND requires funct7=0");
            Executor::execAND(*this, instr);
            break;
        default:
            illegal(instr, "unknown funct3 for <opcode>");
            break;
        }
        break;
    }

    // I-Type Instructions (Immediate)
    case Opcode::I_Type:
        switch (func3)
        {
        case Func3::ADD_SUB:
            Executor::execADDI(*this, instr);
            break;

        case Func3::SLL:
            if (func7 != Func7::ADD_SRL) illegal(instr, "SLLI requires imm[11:5]=0");
            Executor::execSLLI(*this, instr);
            break;

        case Func3::SLT:
            Executor::execSLTI(*this, instr);
            break;

        case Func3::SLTU:
            Executor::execSLTIU(*this, instr);
            break;

        case Func3::XOR:
            Executor::execXORI(*this, instr);
            break;

        case Func3::SRL_SRA:
            switch (func7)
            {
            case Func7::ADD_SRL:
                Executor::execSRLI(*this, instr);
                break;
            case Func7::SUB_SRA:
                Executor::execSRAI(*this, instr);
                break;
            default:
                illegal(instr, "SRLI/SRAI requires imm[11:5]=0 or 0b0100000");
                break;
            }
            break;

        case Func3::OR:
            Executor::execORI(*this, instr);
            break;

        case Func3::AND:
            Executor::execANDI(*this, instr);
            break;
        default:
            illegal(instr, "unknown funct3 for I-type");
            break;
        }
        break;

    // S-Type Instructions (Store)
    case Opcode::STORE:
        switch (func3)
        {
        case Func3::SB:
            Executor::execSB(*this, instr);
            break;
        case Func3::SH:
            Executor::execSH(*this, instr);
            break;
        case Func3::SW:
            Executor::execSW(*this, instr);
            break;
        default:
            illegal(instr, "unknown funct3 for STORE");
            break;
        }
        break;

    // B-Type Instructions (Branch)
    case Opcode::B_Type:
        switch (func3)
        {
        case Func3::BEQ:
            Executor::execBEQ(*this, instr);
            break;
        case Func3::BNE:
            Executor::execBNE(*this, instr);
            break;
        case Func3::BLT:
            Executor::execBLT(*this, instr);
            break;
        case Func3::BGE:
            Executor::execBGE(*this, instr);
            break;
        case Func3::BLTU:
            Executor::execBLTU(*this, instr);
            break;
        case Func3::BGEU:
            Executor::execBGEU(*this, instr);
            break;
        default:
            illegal(instr, "unknown funct3 for BRANCH");
            break;
        }
        break;

    // Load Instructions
    case Opcode::LOAD:
        switch (func3)
        {
        case Func3::LB:
            Executor::execLB(*this, instr);
            break;
        case Func3::LH:
            Executor::execLH(*this, instr);
            break;
        case Func3::LW:
            Executor::execLW(*this, instr);
            break;
        case Func3::LBU:
            Executor::execLBU(*this, instr);
            break;
        case Func3::LHU:
            Executor::execLHU(*this, instr);
            break;
        default:
            illegal(instr, "unknown funct3 for LOAD");
            break;
        }
        break;

    // U-Type & J-Type (Direct Handlers)
    case Opcode::LUI:
        Executor::execLUI(*this, instr);
        break;
    case Opcode::AUIPC:
        Executor::execAUIPC(*this, instr);
        break;
    case Opcode::JAL:
        Executor::execJAL(*this, instr);
        break;
    case Opcode::JALR:
        Executor::execJALR(*this, instr);
        break;

    // System Instructions (ECALL, CSR)
    case Opcode::SYSTEM:
        if (func3 == Func3::ECALL_EBREAK)
        {
            Word imm = Decoder::immI(instr);
            if (imm == 0)
                Executor::execECALL(*this);
            else
                illegal(instr, "SYSTEM imm must be 0 (ECALL)");
            break;
        }
        illegal(instr, "CSR instruction not implemented");
        break;

    // Memory Ordering Instructions (FENCE)
    case Opcode::MISC_MEM:
        switch (func3)
        {
        case Func3::FENCE:
            // NOP for this simulator
            break;
        case Func3::FENCE_I:
            // NOP for this simulator
            break;
        default:
            illegal(instr, "unknown funct3 for MISC_MEM");
            break;
        }
        break;
    default:
        std::cerr << "Unknown Opcode: 0x" << std::hex << (int)op << std::dec << std::endl;
        exit(1);
        break;
    }
}

void Machine::step()
{
    Word instr = this->fetch();
    this->execute(instr);
}

void Machine::dumpRegisters()
{
    std::cout << "PC: 0x" << std::hex << this->pc << "\n";
    for (int i = 0; i < 32; ++i)
    {
        std::cout << "x" << std::dec << i << ": " << std::hex << this->regs[i] << "\n";
    }
}
