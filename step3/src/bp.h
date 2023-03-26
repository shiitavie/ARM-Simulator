/*
 * ARM pipeline timing simulator
 *
 * CMSC 22200, Fall 2021
 */

#ifndef _BP_H_
#define _BP_H_
#include "shell.h"

typedef struct entry {
    uint64_t tag;
    char valid;
    char cond;
    uint64_t targ;
} entry;


typedef struct btb {
    entry entries[1024];
} btb;

typedef struct gshare {
    char PHT[256];
    char GHR;
} gshare;


typedef struct bp_t
{
    gshare bp_gshare;
    btb bp_btb;
} bp_t;


void bp_predict(uint64_t current_pc);
void bp_update(char taken, char cond, uint64_t address, uint64_t current_pc);

#endif
