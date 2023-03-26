
/*
 * Nicole Souydalay (nicolesouydalay)
 * Stevie Xie (zhiyuanxie)
 */
#include <stdio.h>
#include "shell.h"

uint32_t instruction;
char rm, rd, rt, rn;
unsigned int imm12, imm16, imms, immr;
long imm9, imm19, imm26;
unsigned int opcode;

void fetch()
{
    instruction = mem_read_32(CURRENT_STATE.PC);
}

void decode()
{
    // extract all the possible instruction components
    
    
    rm = (instruction >> 16) & 0x1F;
    rn = (instruction >> 5) & 0x1F;
    rd = instruction & 0x1F;
    rt = rd;
    imms = (instruction >> 10) & 0x3F;
    immr = (instruction >> 16) & 0x3F;
    imm9 = (instruction >> 12) & 0x1FF;
    imm12 = (instruction >> 10) & 0xFFF;
    imm16 = (instruction >> 5) & 0xFFFF;
    imm19 = (instruction >> 5) & 0x7FFFF;
    imm9 = (imm9 << 23) >> 23; //sign extension 
    imm19 = (imm19 << 45) >> 45; //sign extension
    imm19 *= 4;
    imm26 = instruction & 0x3FFFFFF;
    imm26 = (imm26 << 38) >> 38;
    imm26 *= 4;
    opcode = instruction >> (32-11); //bit shift to extract raw opcode (11-bit)
}

void addi()
{
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] + imm12;
} 

void add()
{
    NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm]);
} 

void addis()
{
    NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rn] + imm12);
    if (NEXT_STATE.REGS[rd] < 0)
    {
        NEXT_STATE.FLAG_N = 1;
    }
    else 
    {
        NEXT_STATE.FLAG_N = 0;
    }

    if (NEXT_STATE.REGS[rd] == 0)
    {
        NEXT_STATE.FLAG_Z = 1;
    }
    else
    {
        NEXT_STATE.FLAG_Z = 0;
    }
} 

void adds()
{
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rm] + CURRENT_STATE.REGS[rn];

    if (NEXT_STATE.REGS[rd] < 0)
    {
        NEXT_STATE.FLAG_N = 1;
    }
    else {
        NEXT_STATE.FLAG_N = 0;
    }

    if (NEXT_STATE.REGS[rd] == 0)
    {
        NEXT_STATE.FLAG_Z = 1;
    }
    else 
    {
        NEXT_STATE.FLAG_Z = 0;
    }
} 

void cbnz()
{
    if (CURRENT_STATE.REGS[rt]){
        NEXT_STATE.PC = CURRENT_STATE.PC + imm19 - 4;
    }
} 

void cbz()
{
    if (!CURRENT_STATE.REGS[rt]){
        NEXT_STATE.PC = CURRENT_STATE.PC + imm19 - 4;
    }
} 

void subi()
{
    NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rn] - imm12); 
} 

void sub()
{
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm]; 
} 

void subs()
{
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm]; 

    if (NEXT_STATE.REGS[rd] < 0){
        NEXT_STATE.FLAG_N = 1;
    }
    else {
        NEXT_STATE.FLAG_N = 0;
    }

    if (NEXT_STATE.REGS[rd] == 0){
        NEXT_STATE.FLAG_Z = 1;
    }
    else {
        NEXT_STATE.FLAG_Z = 0;
    }
} 

void subsi()
{
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] - imm12; 
    if (NEXT_STATE.REGS[rd] < 0){
        NEXT_STATE.FLAG_N = 1;
    }
    else {
        NEXT_STATE.FLAG_N = 0;
    }

    if (NEXT_STATE.REGS[rd] == 0){
        NEXT_STATE.FLAG_Z = 1;
    }
    else {
        NEXT_STATE.FLAG_Z = 0;
    }
} 

void and()
{
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rm] & CURRENT_STATE.REGS[rn]; 
} 

void eor()
{
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rm] ^ CURRENT_STATE.REGS[rn]; 
} 

void orr()
{
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rm] | CURRENT_STATE.REGS[rn]; 
} 

void ands()
{
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rm] & CURRENT_STATE.REGS[rn];

    if (NEXT_STATE.REGS[rd] < 0)
    {
        NEXT_STATE.FLAG_N = 1;
    }
    else 
    {
        NEXT_STATE.FLAG_N = 0;
    }

    if (NEXT_STATE.REGS[rd] == 0)
    {
        NEXT_STATE.FLAG_Z = 1;
    }
    else {
        NEXT_STATE.FLAG_Z = 0;
    }
} 

void mul()
{
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] * CURRENT_STATE.REGS[rm]; 
} 

void hlt()
{
    RUN_BIT = 0;
}

void ldur_64()
{
    uint64_t value1 = mem_read_32(CURRENT_STATE.REGS[rn] + imm9);
    uint64_t value2 = mem_read_32(CURRENT_STATE.REGS[rn] + imm9 + 4);
    uint64_t value = value1 | (value2 << 32);
    NEXT_STATE.REGS[rt] = value;
}

void ldur_32()
{
    uint32_t value = mem_read_32(CURRENT_STATE.REGS[rn] + imm9);
    NEXT_STATE.REGS[rt] = value;
}

void ldurb()
{
    uint64_t mask = 0 | 0xff; 
    uint64_t value = mask & mem_read_32(CURRENT_STATE.REGS[rn] + imm9);
    NEXT_STATE.REGS[rt] = value;
}

void ldurh()
{
    uint64_t mask = 0 | 0xffff; 
    uint64_t value = mask & mem_read_32(CURRENT_STATE.REGS[rn] + imm9);
    NEXT_STATE.REGS[rt] = value;
}

void stur_64()
{
    uint64_t value = CURRENT_STATE.REGS[rt];
    mem_write_32(CURRENT_STATE.REGS[rn] + imm9, value & 0xFFFFFFFF);
    mem_write_32(CURRENT_STATE.REGS[rn] + imm9 + 4, (value & 0xFFFFFFFF00000000) >> 32);

    mem_write_32(CURRENT_STATE.REGS[rn] + imm9, value);
}

void stur_32()
{
    uint32_t value = CURRENT_STATE.REGS[rt];
    mem_write_32(CURRENT_STATE.REGS[rn] + imm9, value);
}

void sturb()
{
    unsigned int value = CURRENT_STATE.REGS[rt];
    uint64_t address = CURRENT_STATE.REGS[rn] + imm9;
    value = (value & 0xFF) | (mem_read_32(address) & 0xffffff00);
    mem_write_32(address, value);
}

void sturh()
{
    unsigned int value = CURRENT_STATE.REGS[rt];
    uint64_t address = CURRENT_STATE.REGS[rn] + imm9;
    value = (value & 0xffff) | (mem_read_32(address) & 0xffff0000);
    mem_write_32(address, value);
}

void br()
{
    NEXT_STATE.PC = CURRENT_STATE.REGS[rn] - 4;
}

void b()
{
    NEXT_STATE.PC = CURRENT_STATE.PC + imm26 - 4; 
}

void bcond() 
{
    if (((rt == 0x0) && CURRENT_STATE.FLAG_Z) || //BEQ
        ((rt == 0x1) && (!CURRENT_STATE.FLAG_Z)) || //BNE
        ((rt == 0xc) && (!CURRENT_STATE.FLAG_N)) || //BGT
        ((rt == 0xa) && ((!CURRENT_STATE.FLAG_N) || CURRENT_STATE.FLAG_Z)) || //BGE
        ((rt == 0xb) && CURRENT_STATE.FLAG_N) || //BLT
        ((rt == 0xd) && (CURRENT_STATE.FLAG_N || CURRENT_STATE.FLAG_Z))) //BLE
    {
        NEXT_STATE.PC = CURRENT_STATE.PC + imm19 - 4;
    }
}

void ls()
{
    uint64_t to_shift = CURRENT_STATE.REGS[rn];
    if (imms == 0x3f) { //LSR
        NEXT_STATE.REGS[rd] = to_shift >> immr;
    } else {
        NEXT_STATE.REGS[rd] = to_shift << (64-immr);
    }
}

void movz()
{
    NEXT_STATE.REGS[rd] = imm16;
}


void execute_8_bits() {
    if (opcode == 0x91) {
        addi();
    }
    else if (opcode == 0xd1) {
        subi();
    }
    else if (opcode == 0xb1) {
        addis();
    }
    else if (opcode == 0xF1) {
        subsi();
        if(rd == 31){
            NEXT_STATE.REGS[rd] = 0;
        }
    }
    else if (opcode == 0x8a) {
        and();
    }
    else if (opcode == 0xea) {
        ands();
    }
    else if (opcode == 0xca) {
        eor();
    }
    else if (opcode == 0xaa) {
        orr();
    }
    //
    else if (opcode == 0xB5) {
        cbnz();
    }
    else if (opcode == 0xB4) {
        cbz();
    }
    else if (opcode == 0x54) {
        bcond();
    }
    else if (opcode >> 2 == 0x5) {
        b();
    }
}

void execute()
{
    // 11 bit opcodes
    if (opcode == 0x658) {
        sub();
    }
    else if (opcode == 0x458) {
        add();
    }
    else if (opcode == 0x558) {
        adds();
    }
    else if (opcode == 0x758) { //CMP is an alias of subs
        subs();
        if (rd == 31){
            NEXT_STATE.REGS[rd] = 0;
        }
    }
    else if (opcode == 0x4d8) {
        mul();
    }
    else if (opcode == 0x6a2) {
        hlt();
    }
    else if (opcode == 0x6b0) {
        br();
    }
    else if (opcode == 0x7c2) 
    {
        ldur_64();
    }
    else if (opcode == 0x5c2) 
    {
        ldur_32();
    }
    else if (opcode == 0x1C2) 
    {
        ldurb();
    }
    else if (opcode == 0x3C2) 
    {
        ldurh();
    }
    else if (opcode == 0x7c0) 
    {
        stur_64();
    }
    else if (opcode == 0x5c0)
    {
        stur_32();
    }
    else if (opcode == 0x1C0) 
    {
        sturb();
    }
    else if (opcode == 0x3C0) 
    {
        sturh();
    }
    else if (opcode == 0x694) {
        movz();
    }
    else if (opcode >> 1 == 0x34D) { //10 bit opcode
        ls();
    }
    else {
        opcode = opcode >> 3;
        execute_8_bits();
    }    // 8 bit opcodes
    
}


void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */
    fetch();
    decode();
    execute();
    NEXT_STATE.PC += 4;
}
