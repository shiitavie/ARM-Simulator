/*
 * CMSC 22200
 *
 * Nicole Souydalay (nicolesouydalay)
 * Stevie Xie (zhiyuanxie)
 * 
 * ARM pipeline timing simulator
 */

#include "pipe.h"
#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>



/* global pipeline state */
CPU_State CURRENT_STATE;

Pipe_Reg_IFtoDE IFtoDE;
Pipe_Reg_DEtoEX DEtoEX;
Pipe_Reg_EXtoMEM EXtoMEM;
Pipe_Reg_MEMtoWB MEMtoWB;
char if_flag = 1;
char stall_flag = 0;
int count = 1;


void ldurs(uint64_t data_at_rn)
{
    EXtoMEM.mem_flag = 1;
    EXtoMEM.read_flag = 1;
    EXtoMEM.address = data_at_rn + DEtoEX.imm9;
    EXtoMEM.rd = DEtoEX.rt; 
    EXtoMEM.wb_flag = 1;
    switch (DEtoEX.opcode & 0xf00) {
        case 0x700:
            EXtoMEM.byte_count = 8;
            break;
        case 0x500:
            EXtoMEM.byte_count = 4;
            break;
        case 0x300:
            EXtoMEM.byte_count = 2;
            break;
        case 0x100:
            EXtoMEM.byte_count = 1;
            break;
    }
}

void sturs(uint64_t data_at_rn, uint64_t data_at_rt)
{
    EXtoMEM.mem_flag = 1;
    EXtoMEM.write_flag = 1;
    EXtoMEM.wb_flag = 0;
    EXtoMEM.address = data_at_rn + DEtoEX.imm9;
    printf("made it to stur ; %lx \n", DEtoEX.imm9);
    EXtoMEM.value = data_at_rt;
    switch (DEtoEX.opcode & 0xf00) {
        case 0x700:
            EXtoMEM.byte_count = 8;
            break;
        case 0x500:
            EXtoMEM.byte_count = 4;
            break;
        case 0x300:
            EXtoMEM.byte_count = 2;
            break;
        case 0x100:
            EXtoMEM.byte_count = 1;
            break;
    }
}

void br(uint64_t data_at_rn)
{
    stall_flag = 1;
    DEtoEX.void_flag = 2;
    EXtoMEM.address = data_at_rn;
    EXtoMEM.taken_flag = 1;
}

void bcond()
{
    stall_flag = 1;
    DEtoEX.void_flag = 2;
    printf("n_flag = %u \n", EXtoMEM.FLAG_N);
    printf("z_flag = %u \n", EXtoMEM.FLAG_Z);
    
    if (((DEtoEX.rt == 0x0) && EXtoMEM.FLAG_Z) || //BEQ
        ((DEtoEX.rt == 0x1) && (!EXtoMEM.FLAG_Z)) || //BNE
        ((DEtoEX.rt == 0xc) && (!EXtoMEM.FLAG_N)) || //BGT
        ((DEtoEX.rt == 0xa) && ((!EXtoMEM.FLAG_N) || EXtoMEM.FLAG_Z)) || //BGE
        ((DEtoEX.rt == 0xb) && EXtoMEM.FLAG_N) || //BLT
        ((DEtoEX.rt == 0xd) && (EXtoMEM.FLAG_N || EXtoMEM.FLAG_Z))) //BLE
    {
        EXtoMEM.taken_flag = 1;
        EXtoMEM.address = CURRENT_STATE.PC + DEtoEX.imm19;

    } else {
        EXtoMEM.taken_flag = 0;
        //EXtoMEM.address = CURRENT_STATE.PC + 4;
    }
    
}

void b()
{
    stall_flag = 1;
    DEtoEX.void_flag = 2;
    EXtoMEM.taken_flag = 1;
    //printf("branch detected");
    EXtoMEM.address = CURRENT_STATE.PC + DEtoEX.imm26; 
}

void addi(uint64_t data_at_rn)
{
    EXtoMEM.value = data_at_rn + DEtoEX.imm12;
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd;
} 


void addis(uint64_t data_at_rn)
{
    EXtoMEM.value = data_at_rn + DEtoEX.imm12;
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd;

    if (EXtoMEM.value < 0)
    {
        EXtoMEM.FLAG_N = 1;
    }
    else 
    {
        EXtoMEM.FLAG_N = 0;
    }

    if (EXtoMEM.value == 0)
    {
        EXtoMEM.FLAG_Z = 1;
    }
    else
    {
        EXtoMEM.FLAG_Z = 0;
    }
} 

void subsi(uint64_t data_at_rn)
{
    EXtoMEM.value = data_at_rn - DEtoEX.imm12;
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd;

    if (EXtoMEM.value < 0)
    {
        EXtoMEM.FLAG_N = 1;
    }
    else 
    {
        EXtoMEM.FLAG_N = 0;
    }

    if (EXtoMEM.value == 0)
    {
        EXtoMEM.FLAG_Z = 1;
    }
    else
    {
        EXtoMEM.FLAG_Z = 0;
    }
}




void add(uint64_t data_at_rn, uint64_t data_at_rm)
{
    EXtoMEM.value = (data_at_rn + data_at_rm);
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd;
} 

void adds(uint64_t data_at_rn, uint64_t data_at_rm)
{
    EXtoMEM.value = data_at_rm + data_at_rn;
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd; 
    
    if (EXtoMEM.value < 0)
    {
        EXtoMEM.FLAG_N = 1;
    }
    else 
    {
        EXtoMEM.FLAG_N = 0;
    }

    if (EXtoMEM.value == 0)
    {
        EXtoMEM.FLAG_Z = 1;
    }
    else
    {
        EXtoMEM.FLAG_Z = 0;
    }
} 

void subs(uint64_t data_at_rn, uint64_t data_at_rm)
{
    printf("CMP!!\n");
    EXtoMEM.value = data_at_rn - data_at_rm;
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd;

    
    if (EXtoMEM.value < 0)
    {
        EXtoMEM.FLAG_N = 1;
    }
    else 
    {
        EXtoMEM.FLAG_N = 0;
    }

    if (EXtoMEM.value == 0)
    {
        EXtoMEM.FLAG_Z = 1;
    }
    else
    {
        EXtoMEM.FLAG_Z = 0;
    }

} 

void sub(uint64_t data_at_rn, uint64_t data_at_rm)
{
    EXtoMEM.value = data_at_rn - data_at_rm;
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd;
} 

void subi(uint64_t data_at_rn)
{
    EXtoMEM.value = data_at_rn - DEtoEX.imm12;
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd;
} 

void cbz(uint64_t data_at_rt)
{
    stall_flag = 1;
    DEtoEX.void_flag = 2;
    if (!data_at_rt){
        EXtoMEM.address = CURRENT_STATE.PC + DEtoEX.imm19;
        EXtoMEM.taken_flag = 1;
    }
    //else {
       // EXtoMEM.address = CURRENT_STATE.PC + 4;
    //}
} 


void and(uint64_t data_at_rn, uint64_t data_at_rm)
{
    EXtoMEM.value = data_at_rn & data_at_rm; 
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd;
} 

void eor(uint64_t data_at_rn, uint64_t data_at_rm)
{
    EXtoMEM.value = data_at_rn ^ data_at_rm; 
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd;
} 

void orr(uint64_t data_at_rn, uint64_t data_at_rm)
{
    EXtoMEM.value = data_at_rn | data_at_rm; 
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd;
} 

void ands(uint64_t data_at_rn, uint64_t data_at_rm)
{
    EXtoMEM.value = data_at_rm & data_at_rn;
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd;

    if (EXtoMEM.value < 0)
    {
        EXtoMEM.FLAG_N = 1;
    }
    else 
    {
        EXtoMEM.FLAG_N = 0;
    }

    if (EXtoMEM.value == 0)
    {
        EXtoMEM.FLAG_Z = 1;                    
    }
    else {
        EXtoMEM.FLAG_Z = 0;
    }
}

void cbnz(uint64_t data_at_rt)
{
    stall_flag = 1;
    DEtoEX.void_flag = 2;
    if (data_at_rt){
        EXtoMEM.address = CURRENT_STATE.PC + DEtoEX.imm19;
        EXtoMEM.taken_flag = 1;
    } //else {
       //EXtoMEM.address = CURRENT_STATE.PC + 4;
   // }
} 

void mul(uint64_t data_at_rn, uint64_t data_at_rm)
{
    EXtoMEM.value = data_at_rn * data_at_rm;
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd;
}

void movz()
{
    EXtoMEM.rd = DEtoEX.rd;
    EXtoMEM.value = DEtoEX.imm16;
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd;
}

void ls(uint64_t data_at_rn)
{
    uint64_t to_shift = data_at_rn;
    if (DEtoEX.imms == 0x3f) { //LSR
        EXtoMEM.value = to_shift >> DEtoEX.immr;
    } else {
        EXtoMEM.value = to_shift << (64 - DEtoEX.immr);
    }
    EXtoMEM.wb_flag = 1;
    EXtoMEM.rd = DEtoEX.rd;
}

void pipe_init()
{
    memset(&CURRENT_STATE, 0, sizeof(CPU_State));
    CURRENT_STATE.PC = 0x00400000;
    IFtoDE.de_flag = 0;
    DEtoEX.ex_flag = 0;
    EXtoMEM.mem_flag = 0;
    MEMtoWB.wb_flag = 0;
}

void pipe_cycle()
{
	pipe_stage_wb();
	pipe_stage_mem();
	pipe_stage_execute();
	pipe_stage_decode();
	pipe_stage_fetch();
    printf("Count: %d", count);
    count +=1;
}

void pipe_stage_wb()
{
    printf("Final, MEM Value: %lx", MEMtoWB.value);
    printf("Destinaton Register: %d", MEMtoWB.rd);
    printf("Halt Flag: %dx\n", MEMtoWB.hlt_flag);
    if (MEMtoWB.hlt_flag) {
        RUN_BIT = 0;
        stat_inst_retire += 1;
        return;
    }
    if (MEMtoWB.wb_flag) {
        CURRENT_STATE.REGS[MEMtoWB.rd] = MEMtoWB.value;
        CURRENT_STATE.FLAG_N = MEMtoWB.FLAG_N;
        CURRENT_STATE.FLAG_Z = MEMtoWB.FLAG_Z;
        printf("Writing back\n");

    }
    if (MEMtoWB.instr_flag && !MEMtoWB.void_flag) {
        stat_inst_retire += 1;
    }
}

void pipe_stage_mem()
{
    //printf("write_flag = %u \n", EXtoMEM.write_flag);
    //printf("read_flag = %u \n", EXtoMEM.read_flag);
    //printf("byte = %u \n", EXtoMEM.byte_count);
    //printf("addy = %lx \n", EXtoMEM.address);
    //printf("val = %lx \n", EXtoMEM.value);
    uint64_t value2 = mem_read_32(EXtoMEM.address + 4);
    //printf("MEMtoWB.rd = %u \n", MEMtoWB.rd);
    MEMtoWB.FLAG_N = EXtoMEM.FLAG_N;
    MEMtoWB.FLAG_Z = EXtoMEM.FLAG_Z;
    MEMtoWB.rd = EXtoMEM.rd;
    MEMtoWB.wb_flag = EXtoMEM.wb_flag;
    MEMtoWB.value = EXtoMEM.value;
    MEMtoWB.hlt_flag = EXtoMEM.hlt_flag;
    MEMtoWB.void_flag = EXtoMEM.void_flag;

    if (EXtoMEM.hlt_flag) {
        return;
    }
    if (EXtoMEM.mem_flag){
        MEMtoWB.instr_flag = 1;
        printf("%s \n", "mem");
        if (EXtoMEM.write_flag) {
            // Save old contents of the address
            uint64_t old_contents = mem_read_32(EXtoMEM.address);
            printf("Remainder: %ld\n", EXtoMEM.address % 4);
            switch (EXtoMEM.byte_count) {
                
                case(8):
                    printf("Stur 8");
                    printf("%lx", EXtoMEM.address);
                    mem_write_32(EXtoMEM.address, EXtoMEM.value & 0xFFFFFFFF);
                    mem_write_32(EXtoMEM.address + 4, (EXtoMEM.value & 0xFFFFFFFF00000000) >> 32);
                    break;
                case(4):
                    printf("%s", "Stur4");
                    mem_write_32(EXtoMEM.address, EXtoMEM.value);
                    break;
                case(2):
                    printf("%s", "Stur2");
                    if(EXtoMEM.address % 4 == 1)
                    {
                        EXtoMEM.address -= 1;
                        old_contents = mem_read_32(EXtoMEM.address);
                        EXtoMEM.value = (EXtoMEM.value & 0xffff) | (old_contents & 0xffff);
                        mem_write_32(EXtoMEM.address, EXtoMEM.value);
                        EXtoMEM.address += 2;
                        old_contents = mem_read_32(EXtoMEM.address);
                        EXtoMEM.value = ((EXtoMEM.value & 0xffff0000) >> 4) | (old_contents & 0xffff0000);
                        mem_write_32(EXtoMEM.address, EXtoMEM.value);

                    }
                    else if(EXtoMEM.address % 4 == 2)
                    {
                        EXtoMEM.address -= 2;
                        old_contents = mem_read_32(EXtoMEM.address);
                        EXtoMEM.value = ((EXtoMEM.value & 0xffff) << 16) | (old_contents & 0x0000ffff);
                        printf("new val = %lx \n", EXtoMEM.value);
                        mem_write_32(EXtoMEM.address, EXtoMEM.value);
                    }
                    else if(EXtoMEM.address % 4 == 3)
                    {
                        EXtoMEM.address -= 3;
                        old_contents = mem_read_32(EXtoMEM.address);
                        EXtoMEM.value = ((EXtoMEM.value & 0xff) << 24) | (old_contents & 0x00ffffff);
                        mem_write_32(EXtoMEM.address, EXtoMEM.value);
                        EXtoMEM.address += 4;
                        old_contents = mem_read_32(EXtoMEM.address);
                        EXtoMEM.value = ((EXtoMEM.value & 0xff00) >> 8) | (old_contents & 0xffffff00);
                        mem_write_32(EXtoMEM.address, EXtoMEM.value);
                    }
                    else
                    {
                        EXtoMEM.value = (EXtoMEM.value & 0xffff) | (old_contents & 0xffff0000);
                        mem_write_32(EXtoMEM.address, EXtoMEM.value);
                    }
                    break;
                case(1):
                    printf("Stur 1");
                    if(EXtoMEM.address % 4 == 1)
                    {
                        EXtoMEM.address -= 1;
                        old_contents = mem_read_32(EXtoMEM.address);
                        EXtoMEM.value = (old_contents & 0xff) | ((EXtoMEM.value & 0xff) << 8) | (old_contents & 0xFFFF0000);
                        mem_write_32(EXtoMEM.address, EXtoMEM.value);
                    }
                    else if(EXtoMEM.address % 4 == 2)
                    {
                        EXtoMEM.address -= 2;
                        old_contents = mem_read_32(EXtoMEM.address);
                        printf("New contents: %lx \n", EXtoMEM.value);
                        printf("Old contents: %lx \n", old_contents);
                        EXtoMEM.value = (old_contents & 0xffff) | ((EXtoMEM.value & 0xff) << 16) | (old_contents & 0xFF000000);
                        mem_write_32(EXtoMEM.address, EXtoMEM.value);
                        printf("new val = %lx \n", EXtoMEM.value);
                    }
                    else if(EXtoMEM.address % 4 == 3)
                    {
                        EXtoMEM.address -= 3;
                        old_contents = mem_read_32(EXtoMEM.address);
                        EXtoMEM.value = (old_contents & 0xffffff) | ((EXtoMEM.value & 0xff) << 24);
                        mem_write_32(EXtoMEM.address, EXtoMEM.value);
                    }
                    else
                    {
                        EXtoMEM.value = (EXtoMEM.value & 0xff) | (old_contents & 0xffffff00);
                        mem_write_32(EXtoMEM.address, EXtoMEM.value);
                    }
                    break;
            }
        }
        else if (EXtoMEM.read_flag){
            MEMtoWB.value = mem_read_32(EXtoMEM.address);
            printf("Expected output: %lx", MEMtoWB.value);
            switch (EXtoMEM.byte_count) {
                case(8):
                    MEMtoWB.value = MEMtoWB.value | (value2 << 32);
                    printf("Expected output8: %lx", MEMtoWB.value);
                    break;
                case(4):
                    printf("Expected output4: %lx", MEMtoWB.value);
                    break;
                case(2):
                    if(EXtoMEM.address % 4 == 1)
                    {
                        EXtoMEM.address -= 1;
                        MEMtoWB.value = (mem_read_32(EXtoMEM.address) & 0xffff00) >> 8;
                    }
                    else if(EXtoMEM.address % 4 == 2)
                    {
                        EXtoMEM.address -= 2;
                        MEMtoWB.value = (mem_read_32(EXtoMEM.address) & 0xffff0000) >> 16;
                    }
                    else if(EXtoMEM.address % 4 == 3)
                    {
                        EXtoMEM.address -= 3;
                        MEMtoWB.value = (mem_read_32(EXtoMEM.address) & 0xff000000) >> 24;
                        EXtoMEM.address += 4;
                        MEMtoWB.value = MEMtoWB.value | (mem_read_32(EXtoMEM.address & 0xff) << 8);
                    }
                    else
                    {
                        MEMtoWB.value = MEMtoWB.value & 0xffff;
                    }
                    printf("Expected output2: %lx", MEMtoWB.value);
                    break;
                case(1):
                    if(EXtoMEM.address % 4 == 1)
                    {
                        EXtoMEM.address -= 1;
                        MEMtoWB.value = (mem_read_32(EXtoMEM.address) & 0xff00) >> 8;
                        printf("Case 1");
                    }
                    else if(EXtoMEM.address % 4 == 2)
                    {
                        EXtoMEM.address -= 2;
                        MEMtoWB.value = (mem_read_32(EXtoMEM.address) & 0x00ff0000) >> 16;
                        printf("Case 2");
                    }
                    else if(EXtoMEM.address % 4 == 3)
                    {
                        EXtoMEM.address -= 3;
                        MEMtoWB.value = (mem_read_32(EXtoMEM.address) & 0xff000000) >> 24;
                        printf("Case 3");
                    }
                    else
                    {
                        MEMtoWB.value = MEMtoWB.value & 0xff;
                        printf("Case 4");
                    }
                    printf("Expected output1: %lx", MEMtoWB.value);
                    break;
            }
        }
        //printf("MEMtoWB.rd = %u \n", MEMtoWB.rd);
        //printf("MEMtoWB.value = %lu \n", MEMtoWB.value);
        //printf("MEMtoWB.wb_flag = %u \n", MEMtoWB.wb_flag);
    } 
    
    //if (!EXtoMEM.mem_flag){
    //    return;
    //}
    
    
    
}

void pipe_stage_execute()
{
    printf("opcode = %x \n", DEtoEX.opcode);
    if (DEtoEX.void_flag) {
        EXtoMEM.void_flag = DEtoEX.void_flag;
        DEtoEX.void_flag -= 1; 
        stall_flag = 0;
        return;
    }
    if (!DEtoEX.ex_flag)
    {
        return;
    }
    if (DEtoEX.opcode == 0x6a2) {
        if_flag = 0;
        EXtoMEM.hlt_flag = 1;
        return;
    } else {
        EXtoMEM.hlt_flag = 0;
    }	

    uint64_t data_at_rm = CURRENT_STATE.REGS[DEtoEX.rm];
    uint64_t data_at_rn = CURRENT_STATE.REGS[DEtoEX.rn];
    uint64_t data_at_rt = CURRENT_STATE.REGS[DEtoEX.rt];
    
    if(stall_flag) {
        stall_flag = 0;
    } else {
        if(DEtoEX.rm == EXtoMEM.rd)
        {
            data_at_rm = EXtoMEM.value;
            if (EXtoMEM.read_flag == 1 && EXtoMEM.write_flag == 0) {
                stall_flag = 1;
                EXtoMEM.void_flag = 1;
                return;
            }        
        }
        else if(DEtoEX.rm == MEMtoWB.rd)
        {
            data_at_rm = MEMtoWB.value;
        }

        if(DEtoEX.rn == EXtoMEM.rd)
        {
            data_at_rn = EXtoMEM.value;
            if (EXtoMEM.read_flag == 1 && EXtoMEM.write_flag == 0) {
                stall_flag = 1;
                EXtoMEM.void_flag = 1;
                return;
            }        
        }
        else if(DEtoEX.rn == MEMtoWB.rd)
        {
            data_at_rn = MEMtoWB.value;
        }
        if(DEtoEX.rt == MEMtoWB.rd)
        {
            data_at_rt = MEMtoWB.value;
        }
    }

    if (EXtoMEM.void_flag) {
        EXtoMEM.void_flag = 0;
    }

    EXtoMEM.mem_flag = 0;
    EXtoMEM.read_flag = 0;
    EXtoMEM.write_flag = 0;
    EXtoMEM.taken_flag = 0;
    //EXtoMEM.hlt_flag = DEtoEX.hlt_flag;
    if (DEtoEX.opcode == 0x658) {
        sub(data_at_rn, data_at_rm);
    }
    else if (DEtoEX.opcode == 0x458) {
        add(data_at_rn, data_at_rm);
    }
    else if (DEtoEX.opcode == 0x558) {
        adds(data_at_rn, data_at_rm);
    } 
    else if ((DEtoEX.opcode&0xff) == 0xc2) {
        ldurs(data_at_rn);
    }
    else if ((DEtoEX.opcode&0xff) == 0xc0) {
        sturs(data_at_rn, data_at_rt);
    }
    else if (DEtoEX.opcode == 0x758) { //CMP is an alias of subs
        subs(data_at_rn, data_at_rm);
        if (DEtoEX.rd == 31){
            EXtoMEM.value = 0;
            EXtoMEM.wb_flag = 1;
        }
    }
    else if (DEtoEX.opcode == 0x4d8) {
        mul(data_at_rn, data_at_rm);
    }
    else if (DEtoEX.opcode == 0x694) {
        movz();
    }
    else if (DEtoEX.opcode >> 1 == 0x34D) { //10 bit opcode
        ls(data_at_rn);
    }

    // 8 bit opcodes
    DEtoEX.opcode = DEtoEX.opcode >> 3;

    if (DEtoEX.opcode == 0x8a) {
        and(data_at_rn, data_at_rm);
    }
    else if (DEtoEX.opcode == 0xea) {
        ands(data_at_rn, data_at_rm);
    }
    else if (DEtoEX.opcode == 0xca) {
        eor(data_at_rn, data_at_rm);
    }
    else if (DEtoEX.opcode == 0xaa) {
        orr(data_at_rn, data_at_rm);
    }
	else if (DEtoEX.opcode == 0x6b0) {
        br(data_at_rn);
    }
    else if (DEtoEX.opcode == 0x54) {
        bcond(); 
    }
    else if (DEtoEX.opcode == 0x91) {
        addi(data_at_rn);
    }
    else if (DEtoEX.opcode == 0xb1) {
        addis(data_at_rn);
        return;
    }
    else if (DEtoEX.opcode == 0xd1) {
        subi(data_at_rn);
    }
    else if (DEtoEX.opcode == 0xB4) {
        cbz(data_at_rt);
    }
    else if (DEtoEX.opcode == 0xF1) {
        subsi(data_at_rn);
        if(DEtoEX.rd == 31){
            EXtoMEM.value = 0;
            EXtoMEM.wb_flag = 1;
        }
    }
    else if (DEtoEX.opcode == 0xB5) {
        cbnz(data_at_rt);
    }
    else if (DEtoEX.opcode >> 2 == 0x5) {
        b();
    }
	
    EXtoMEM.mem_flag = 1;
    //printf("EXtoMEM.rd = %u \n", EXtoMEM.rd);
    //printf("EXtoMEM.value = %lu \n", EXtoMEM.value);
    //printf("EXtoMEM.wb_flag = %u \n", EXtoMEM.wb_flag);
}

void pipe_stage_decode()
{
    if (stall_flag || (!IFtoDE.de_flag || !if_flag)) {
        return;
    }    
    // extract all the possible instruction components
    
    DEtoEX.rm = (IFtoDE.instruction >> 16) & 0x1F;
    DEtoEX.rn = (IFtoDE.instruction >> 5) & 0x1F;
    DEtoEX.rd = IFtoDE.instruction & 0x1F;
    DEtoEX.rt = DEtoEX.rd;
    DEtoEX.imms = (IFtoDE.instruction >> 10) & 0x3F;
    DEtoEX.immr = (IFtoDE.instruction >> 16) & 0x3F;
    DEtoEX.imm9 = (((IFtoDE.instruction >> 12) & 0x1FF) << 23) >> 23;
    DEtoEX.imm12 = (IFtoDE.instruction >> 10) & 0xFFF;
    DEtoEX.imm16 = (IFtoDE.instruction >> 5) & 0xFFFF;
    uint64_t instruction = IFtoDE.instruction;
    DEtoEX.imm19 = ((((instruction >> 5) & 0x7FFFF) << 45) >> 45) * 4;
    DEtoEX.imm26 = (((instruction & 0x3FFFFFF) << 38) >> 38) * 4;
    DEtoEX.opcode = IFtoDE.instruction >> (32-11); //bit shift to extract raw opcode (11-bit)	
    DEtoEX.ex_flag = 1;	
}

void pipe_stage_fetch()
{
	if (stall_flag || !if_flag) {
        if (DEtoEX.void_flag == 2) {
            if (EXtoMEM.taken_flag) {
                if (EXtoMEM.address-4 == CURRENT_STATE.PC){
                DEtoEX.void_flag = 1;
                }
                CURRENT_STATE.PC = EXtoMEM.address - 8;
            }
            else {
                DEtoEX.void_flag = 1;
            }
        } 
        return;
    }

    IFtoDE.instruction = mem_read_32(CURRENT_STATE.PC);
    CURRENT_STATE.PC += 4;
    
    IFtoDE.de_flag = 1;
    printf("instruction: %x \n", IFtoDE.instruction);
}
