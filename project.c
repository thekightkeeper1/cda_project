#include "spimcore.h"
#define MIN_MEM 0x0000
#define MAX_MEM 0xFFFF

#define FUNCT_ADD 0b100000
#define FUNCT_SUB 0b100010
#define FUNCT_SLT 0b101010
#define FUNCT_SLTU 0b101011
#define FUNCT_AND 0b100100
#define FUNCT_OR 0b100101
#define FUNCT_SLL 0b000000
#define FUNCT_NOT 0b100111

#define ALU_ADD 0b000
#define ALU_SUB 0b001
#define ALU_SLT 0b010
#define ALU_SLTU 0b011
#define ALU_AND 0b100
#define ALU_OR 0b101
#define ALU_SLL 0b110
#define ALU_NOT 0b111


unsigned signed2_to_unsigned(unsigned input) {
    const unsigned mask = 0b1 << 31;
    const unsigned maskCalc = input & mask;

    if (maskCalc != 0) {
        // Negative number, need to convert to unsigned representaion
        input = ~input + 0b1;
    }
    return input;
}

/* ALU */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero) {
    switch (ALUControl) {
        case ALU_ADD:
            // Z = A + B.
            *ALUresult = A + B;
            break;

        case ALU_SUB:
            // Z = A - B.
            *ALUresult = A - B;
            break;

        case ALU_SLT:
            // If A < B, Z = 1; Otherwise, Z = 0.
            A = signed2_to_unsigned(A);
            B = signed2_to_unsigned(B);

            if (A < B) {
                *ALUresult = 1;
            } else {
                *ALUresult = 0;
            }
            break;

        case 0b011:
            // If A < B, Z = 1. Otherwise, Z = 0 (A and B are unsigned).
            if (A < B) {
                *ALUresult = 1;
            } else {
                *ALUresult = 0;
            }
            break;

        case 0b100:
            // Z = A AND B
            *ALUresult = A & B;
            break;

        case 0b101:
            // Z = A OR B
            *ALUresult = A | B;
            break;

        case 0b110:
            // Z = B << 16
            *ALUresult = B << 16;
            break;

        case 0b111:
            // Z = ~A
            *ALUresult = ~A;
        default: ;
    }


    // Setting the zero value incase something needed it
    if (*ALUresult == 0)
        *Zero = 1;
    else
        *Zero = 0;
}


/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction) {
    // Checking for word alignment
    if ((PC % 4) != 0) {
        return 1; // HALT, the PC is not word aligned
    }
    // Check out-of-bounds
    // Since PC is unsigned we don't *have* to check if its less, but who knows?
    if (PC < MIN_MEM || PC > MAX_MEM)
        return 1; // HALT, out of bounds

    *instruction = Mem[PC >> 2]; // Mem is array of words, but PC is in bytes, so we need to convert PC by dividing by 4
    return 0; // Success, don't halt
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3,
                           unsigned *funct, unsigned *offset, unsigned *jsec) {
    // Making bitmasks to obtain relavant bits from the instrutions
    unsigned opM = 0x3f << 26; // opcode = [31:26]
    unsigned r1M = 0x1f << 21; // read_reg1 = [25:21]
    unsigned r2M = 0x1f << 16; // read_reg2 or write_reg = [20:16]
    unsigned r3M = 0x1f << 11; // write_reg = [15:11]
    unsigned jsecM = 0x3ffffff; // jump_addr = [25:0]
    unsigned offsetM = 0xffff; // sign_extended = [15:0]
    unsigned functM = 0x3f; // r_type_funct = [5:0]

    // Bitmasking and writing those bits
    // We also have to right shift all the extra 0 out of some
    *op = (instruction & opM) >> 26;
    *r1 = (instruction & r1M) >> 21;
    *r2 = (instruction & r2M) >> 16;
    *r3 = (instruction & r3M) >> 11;
    *jsec = instruction & jsecM;
    *offset = instruction & offsetM;
    *funct = instruction & functM;
}


/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op, struct_controls *controls) {
    // This reads the OP and assigns the control_signals
    // The 14 instructions we actually have to use are at the bottom of project guidlines
    // Also study set 6 to help. And mips reference

    switch (op) {
        case 0b000000: // R-type instruction (add, sub, or, and, slt, sltu)
            controls->RegDst = 0b1; // want to write to register in bit [15:11] for all R type
            controls->Jump = 0b0; // not jumping
            controls->Branch = 0b0; // not branching
            controls->MemRead = 0b0; // not reading memory
            controls->MemtoReg = 0b0; // not reading mem
            controls->ALUOp = 0b111; // don't know yet, look at our funct bit [5:0]
            controls->MemWrite = 0b0; // not writing to memory
            controls->ALUSrc = 0b0; // read data 2 to alu for R types
            controls->RegWrite = 0x1; // write data to register
            break;
        case 0b100011: // I-Type load word
            controls->RegDst = 0b0; // Set the mux to read from bit [20:16]for all I-type
            controls->Jump = 0b0; // not jumping
            controls->Branch = 0b0; // not branching
            controls->MemRead = 0b1; // reading memory (like ofc we are)
            controls->MemtoReg = 0b1; // Set mux to output mem instead of alu to the register file
            controls->ALUOp = 0b0; // Set the
            controls->MemWrite = 0b0; // not writing to memory
            controls->ALUSrc = 0b1; // ALU set to sign-extended for all I type
            controls->RegWrite = 0x1; // Enable reg-file writing
            break;
        case 0b101011: // I-Type store word
            controls->RegDst = 0b0; // Set the mux to read from bit [20:16] for all I-type
            controls->Jump = 0b0; // not jumping
            controls->Branch = 0b0; // not branching
            controls->MemRead = 0b0; // Not reading mem
            controls->MemtoReg = 0b0; // Not reading mem
            controls->ALUOp = 0b0; // Don't care
            controls->MemWrite = 0b1; // Writing to memory
            controls->ALUSrc = 0b1; // ALU set to sign-extended for all I type
            controls->RegWrite = 0b0; // Not writing to reg file
            break;
        case 0b001111: // I-Type load upper immediate
            controls->RegDst = 0b0; // Set the mux to read from bit [20:16] (0b0) for all I-type
            controls->Jump = 0b0; // not jumping
            controls->Branch = 0b0; // not branching
            controls->MemRead = 0b0; // Not reading mem
            controls->MemtoReg = 0b0; // Not reading mem
            controls->ALUOp = 0b110; // Left Shift the value 16
            controls->MemWrite = 0b0; // Not writing to mem
            controls->ALUSrc = 0b1; // ALU set to sign-extended for all I type
            controls->RegWrite = 0x1; // Writing the immediate value
            break;

        case 0b000100: // I-Type branch if equal
            controls->RegDst = 0b0; // Set the mux to read from bit [20:16] (0b0) for all I-type
            controls->Jump = 0b0; // not jumping
            controls->Branch = 0b1; // Branching should be enabled
            controls->MemRead = 0b0; // Not reading mem
            controls->MemtoReg = 0b0; // Not reading mem
            controls->ALUOp = 0b001; // Subtraction to check for equality
            controls->MemWrite = 0b0; // Not writing to mem
            controls->ALUSrc = 0b0; // ALU input mux set to read reg 2 for I-type beq ONLY
            controls->RegWrite = 0b0; // Not writing to registers
            break;
        case 0b001010: // I type set less than immediate
            controls->RegDst = 0b0; // Set the mux to read from bit [20:16] (0b0) for all I-type
            controls->Jump = 0b0; // not jumping
            controls->Branch = 0b0; // Not branching
            controls->MemRead = 0b0; // Not reading mem
            controls->MemtoReg = 0b0; // Not reading mem
            controls->ALUOp = 0b010; // Set less than
            controls->MemWrite = 0b0; // Not writing to mem
            controls->ALUSrc = 0b1; // ALU set to sign-extended for all I type (not beq)
            controls->RegWrite = 0b1; // Writing result to reg
            break;
        case 0b001011: // I-type set less than immediate unsigned
            controls->RegDst = 0b0; // Set the mux to read from bit [20:16] (0b0) for all I-type
            controls->Jump = 0b0; // not jumping
            controls->Branch = 0b0; // Not branching
            controls->MemRead = 0b0; // Not reading mem
            controls->MemtoReg = 0b0; // Not reading mem
            controls->ALUOp = 0b011; // Set less than unsigned
            controls->MemWrite = 0b0; // Not writing to mem
            controls->ALUSrc = 0b1; // ALU set to sign-extended for all I type (not beq)
            controls->RegWrite = 0b1; // Writing result to reg
            break;
        case 0b000010: // J-type jump
            controls->RegDst = 0b10; // Writing is disable therefore dont care
            controls->Jump = 0b1; // Jumping
            controls->Branch = 0b0; // Not branching
            controls->MemRead = 0b0; // Not reading mem
            controls->MemtoReg = 0b10; // Writing to reg is disabled therefore dont care
            controls->ALUOp = 0b000; // Don't care for alu
            controls->MemWrite = 0b0; // Not writing to mem
            controls->ALUSrc = 0b10; // Not using ALU so dont care
            controls->RegWrite = 0b0; // Not writing to reg
            break;
        case 0b001000: // I-type add immediate
            controls->RegDst = 0b0; // Set the mux to read from bit [20:16] (0b0) for all I-type
            controls->Jump = 0b0; // Not jumping
            controls->Branch = 0b0; // Not branching
            controls->MemRead = 0b0; // Writing should come from the alu
            controls->MemtoReg = 0b0; // Not reading mem
            controls->ALUOp = 0b000; // Add operation for the alu
            controls->MemWrite = 0b0; // Not writing to mem
            controls->ALUSrc = 0b1; // ALU set to sign-extended for all I type (not beq)
            controls->RegWrite = 0b1; // Allow writing alu result to reg
            break;


        default: // Not a valid op, so halt
            return 1;
    }
    return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2) {
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}


void sign_extend(unsigned offset, unsigned *extended_value) {
    //If negative, fill with 1s.
    if (offset >> 15 == 1) {
        *extended_value = 0xffff0000 | offset;
    } else //If positive, fill with 0s.
    {
        *extended_value = 0x00000000 | offset;
    }
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc,
                   unsigned *ALUresult, char *Zero) {
    if (ALUOp == 0b111) {
        // Must be R type, so we decide alu operation from funct
        switch (funct) {
            case FUNCT_ADD:
                ALUOp = ALU_ADD;
                break;
            case FUNCT_SUB: // Subtract
                ALUOp = ALU_SUB;
                break;
            case FUNCT_SLT:
                ALUOp = ALU_SLT;
                break;
            case FUNCT_SLTU:
                ALUOp = ALU_SLTU;
                break;
            case FUNCT_AND:
                ALUOp = ALU_AND;
                break;
            case FUNCT_OR:
                ALUOp = ALU_OR;
                break;
            case FUNCT_SLL:
                ALUOp = ALU_SLL;
                break;
            case FUNCT_NOT:
                ALUOp = ALU_NOT;
                break;
        }
    }

    // Deciding ALU inputs based on ALU src.
    // There is no mux for the first input, so that is constant
    const unsigned a = data1;
    unsigned b = data2; // Default or don't care value
    if (ALUSrc == 0b1) b = extended_value;

    ALU(a, b, ALUOp, ALUresult, Zero);
    return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem) {
    //This just makes sure that the memory address is word-aligned.
    //(MIPS handles memory in 4-byte words.)
    //If this case isn't true, then that means everything that happens below
    //is voided.

    if (ALUresult % 4 != 0 && (MemRead == 0b1 || MemWrite == 0b1)) {
        // Halt if we are supposed to do rw operations
        // But we are not word aligned
        return 1;
    }
    //If we're in Read mode.
    if (MemRead == 1 && ALUresult % 4 == 0) {
        *memdata = Mem[ALUresult >> 2]; // Divide by for to get the index of the word
    }

    //If we're in Write mode.
    if (MemWrite == 1 && ALUresult % 4 == 0) {
        Mem[ALUresult >> 2] = data2; // Divide by four to get the index of the word
    }
    return 0;
}


//* Write Register */
/* 10 Points */
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst,
                    char MemtoReg, unsigned *Reg) {
    if (RegWrite == 1) {
        unsigned writingIndex = r2;
        unsigned resultValue = ALUresult;
        if (MemtoReg == 1) // Mux for bringing data from mem or reg
        {
            resultValue = memdata;
        }
        if (RegDst == 1) // Mux for writing to the bits given by r2, or r3
        {
            writingIndex = r3;
        }

        Reg[writingIndex] = resultValue; // Not word aligning. E.g. r8 == 0x8 == Reg[8]
    }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero, unsigned *PC) {
    // Incrementing counter
    if (Branch == 0x0 && Jump == 0x0) *PC += 4;
    else if (Jump) {
        // First we bitmask to keep only the first 4 bits of PC
        // Then we shift jsec by 2, which word-aligns it.
        // Lastly, we combine the two numbers with a bitwise or
        *PC = ((*PC + 4) & 0xf << 28) | jsec << 2;
    } else if (Branch) {
        *PC += 4; // Always do this before branch or jump
        *PC += extended_value << 2; // Word aligning the branch distance, then adding it to PC
    }
}
