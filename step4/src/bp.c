/*
 * ARM pipeline timing simulator
 *
 * CMSC 22200, Fall 2021
 *
 * Nicole Souydalay (nicolesouydalay)
 * Stevie Xie (zhiyuanxie)
 */

#include "bp.h"
#include "shell.h"
#include "pipe.h"
#include <stdio.h>

bp_t branch_predictor; 

void bp_predict(uint64_t current_pc)
{
    char curr_GHR = branch_predictor.bp_gshare.GHR & 0xff;
    char BTB_index = (current_pc & 0xFFF) >> 2;
    entry BTB_entry = branch_predictor.bp_btb.entries[(current_pc & 0xFFF) >> 2];
    uint64_t predicted_address;
    char PHT_index = (curr_GHR ^ ((current_pc & 0x3FF) >> 2));
    char PHT_entry = branch_predictor.bp_gshare.PHT[PHT_index&0xff];
    
    printf("#### bp_predict: \n");
    printf("PHT entry at 0Xa5: %x \n", branch_predictor.bp_gshare.PHT[0xa5]);
    printf("curr_GHR: %x \n", curr_GHR);
    printf("PHT_ENTRY_index: %x \n", PHT_index);
    printf("PHT_ENTRY: %x \n", PHT_entry);
    printf("Index: %u, PC: %lu, Dest: %lu, Valid: %d, Iscond: %d \n", BTB_index, BTB_entry.tag, BTB_entry.targ,BTB_entry.valid, BTB_entry.cond);
    if (BTB_entry.tag != current_pc || BTB_entry.valid == 0)
    {
        predicted_address = current_pc + 4;
        IFtoDE.taken_flag = 0;
    }
    else 
    {
        if (BTB_entry.cond == 0 || (PHT_entry >= 2))
        {
            printf("curr_ghr: %x \n", curr_GHR);
            predicted_address = BTB_entry.targ;
            printf("Predicted: %lx \n", predicted_address);
            IFtoDE.taken_flag = 1;
        }
        else
        {
            predicted_address = current_pc + 4;
            IFtoDE.taken_flag = 0;
        }
        
    }
    CURRENT_STATE.PC = predicted_address;
}

void bp_update(char taken_flag, char cond, uint64_t targ, uint64_t branch_pc)
{
    printf("PHT entry at 0Xa5: %x \n", branch_predictor.bp_gshare.PHT[0xa5]);
    char curr_GHR = branch_predictor.bp_gshare.GHR;
    unsigned int BTB_index = (branch_pc & 0xFFF) >> 2;
    char PHT_index = (curr_GHR ^ ((branch_pc & 0x3FF) >> 2));
    char PHT_entry = branch_predictor.bp_gshare.PHT[PHT_index & 0xff];
    PHT_entry = PHT_entry & 3;
    /* Update BTB */
    branch_predictor.bp_btb.entries[BTB_index].tag = branch_pc;
    branch_predictor.bp_btb.entries[BTB_index].valid = 1;
    branch_predictor.bp_btb.entries[BTB_index].cond = cond;
    branch_predictor.bp_btb.entries[BTB_index].targ = targ;


    /* Update gshare directional predictor */

    if(cond)
    {
        printf("### UPDATING G SHARE!!!! \n");
        printf("TAKEN_FLAG: %x \n", taken_flag);
        printf("PHT_ENTRY_index: %x \n", PHT_index);
        printf("OLD PHT_entry: %x\n", PHT_entry);
        printf("BTB index: %x\n", BTB_index);

        if (taken_flag)
        {
            if(PHT_entry != 3)
                branch_predictor.bp_gshare.PHT[PHT_index & 0xff] = (PHT_entry + 1) & 3;
        }
        else
        {
            if(PHT_entry != 0)
                branch_predictor.bp_gshare.PHT[PHT_index & 0xff] = (PHT_entry - 1) & 3;       
        }
        printf("NEW PHT_entry: %x\n", branch_predictor.bp_gshare.PHT[PHT_index]);
        printf("OLD GHR: %x\n", branch_predictor.bp_gshare.GHR);
        branch_predictor.bp_gshare.GHR = (branch_predictor.bp_gshare.GHR << 1) + taken_flag;
        printf("NEW GHR: %x\n", branch_predictor.bp_gshare.GHR);
    }
}
