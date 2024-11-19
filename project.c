#include "spimcore.h"
#define MIN_MEM 0x0000
#define MAX_MEM 0xFFFF

unsigned signed2_to_unsigned(unsigned input);

unsigned signed2_to_unsigned(unsigned input) {
    const unsigned mask = 0b1 << 31;
    const unsigned maskCalc = input & mask;

    if (maskCalc != 0) {  // Negative number, need to convert to unsigned representaion
        input = ~input + 0b1;
    }
        return input;
}

/* ALU */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero) {
    switch (ALUControl) {
        case 0b000:
            // Z = A + B.
            *ALUresult = A + B;
            break;

        case 0b001:
            // Z = A - B.
            *ALUresult = A - B;
            break;

        case 0b010:
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
        *Zero = 0;
    else
        *Zero = 1;
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

    *instruction = Mem[PC>>2]; // Mem is array of words, but PC is in bytes, so we need to convert PC by dividing by 4
    return 0; // Success, don't halt
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3,
                           unsigned *funct, unsigned *offset, unsigned *jsec) {
}


/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op, struct_controls *controls) {
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2) {
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset, unsigned *extended_value) {
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc,
                   unsigned *ALUresult, char *Zero) {
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem) {
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst,
                    char MemtoReg, unsigned *Reg) {
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
