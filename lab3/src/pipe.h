/*
 * CMSC 22200
 *
 * ARM pipeline timing simulator
 */

#ifndef _PIPE_H_
#define _PIPE_H_

#include "shell.h"
#include "stdbool.h"
#include <limits.h>


typedef struct CPU_State {
	/* register file state */
	int64_t REGS[ARM_REGS];
	int FLAG_N;        /* flag N */
	int FLAG_Z;        /* flag Z */

	/* program counter in fetch stage */
	uint64_t PC;
	
} CPU_State;

typedef struct Pipe_Reg_IFtoDE {
	uint32_t instruction;
	char de_flag, taken_flag;
	uint64_t branch_pc, predicted_pc;
} Pipe_Reg_IFtoDE;

typedef struct Pipe_Reg_DEtoEX {	
	char rm, rd, rt, rn;
	int imm12, imm16, imms, immr;
	long imm9, imm19, imm26;
	unsigned int opcode;
	char ex_flag, hlt_flag, void_flag, taken_flag;
	uint64_t branch_pc, predicted_pc;
} Pipe_Reg_DEtoEX;

typedef struct Pipe_Reg_EXtoMEM {
	char mem_flag, wb_flag, hlt_flag, read_flag, write_flag, void_flag, taken_flag;
	uint64_t address;
	char rd; 
	char byte_count;
	int64_t value;
	int FLAG_N;   
	int FLAG_Z;   
} Pipe_Reg_EXtoMEM;

typedef struct Pipe_Reg_MEMtoWB {
	char wb_flag, hlt_flag, instr_flag, void_flag;
	char rd;
	uint64_t value;	
	int FLAG_N;   
	int FLAG_Z;   
} Pipe_Reg_MEMtoWB;

int RUN_BIT;

/* global variable -- pipeline state */
extern CPU_State CURRENT_STATE;

/* called during simulator startup */
void pipe_init();

/* this function calls the others */
void pipe_cycle();

/* each of these functions implements one stage of the pipeline */
void pipe_stage_fetch();
void pipe_stage_decode();
void pipe_stage_execute();
void pipe_stage_mem();
void pipe_stage_wb();

CPU_State CURRENT_STATE;
Pipe_Reg_IFtoDE IFtoDE;
Pipe_Reg_DEtoEX DEtoEX;
Pipe_Reg_EXtoMEM EXtoMEM;
Pipe_Reg_MEMtoWB MEMtoWB;
#endif
