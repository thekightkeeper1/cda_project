#include "spimcore.h"
#define MIN_MEM 0x0000
#define MAX_MEM 0xFFFF

/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{

}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    // Checking for word alignment
    if ((PC % 4) != 0) {
        return 1; // HALT, the PC is not word aligned
    }
    // Check out-of-bounds
    if (PC > MAX_MEM || PC < MIN_MEM)
        return 1; // HALT, out of bounds
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{

}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{

}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{

}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{

}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{

}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{

}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{

}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    // Incrementing counter
    if (Branch == 0x0 && Jump == 0x0) *PC += 4;
    else if (Jump) {
        // First we bitmask to keep only the first 4 bits of PC
        // Then we shift jsec by 2, which word-aligns it.
        // Lastly, we combine the two numbers with a bitwise or
        *PC = ((*PC+4) & 0xf<<28) | jsec<<2;

    } else if (Branch) {
        *PC += 4; // Always do this before branch or jump
        *PC += extended_value<<2; // Word aligning the branch distance, then adding it to PC
    }
}

