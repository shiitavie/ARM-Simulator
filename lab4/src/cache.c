/*
 * CMSC 22200, Fall 2021
 *
 * Nicole Souydalay (nicolesouydalay)
 * Stevie Xie (zhiyuanxie)
 * 
 * ARM pipeline timing simulator
 *
 */

#include "cache.h"
#include <stdlib.h>
#include "pipe.h"
#include <stdio.h>

// ### instruction cache - 4 way
//[4:0] -> 5 bits as block offset
//[10:5] -> 6 bits as set index
//[63:11] -> 53 bits as tag?  
//each block 32 bytes -> 8 instructions

// ### data cache - 8 way & write-thru and allocate on write 
//[4:0] -> 5 bits as block offset
//[12:5] -> 8 bits as set index
//[63:13] -> 50 bits as tag?  

//defining custom pow and log 

int pow_custom(int x){
    if (!x) {
        return 1;
    } else {
        return 2*pow_custom(x-1);
    }
}

int log_custom(int x){
    if (x == 1) {
        return 0;
    } else {
        return 1+log_custom(x/2);
    }
}

cache_t *cache_new(int sets, int ways, int block)
{
    cache_t* new_cache = (cache_t*)(malloc(sizeof(cache_t)));
    uint64_t max = 0xFFFFFFFFFFFFFFFF;
    int set_offset = log_custom(sets);
    
    //new_cache.num_bytes = block / 8; //????
    int block_offset = log_custom(block);
    int tag_size = 64 - block_offset - set_offset;
    new_cache->tag_mask = ~(max >> tag_size);
    new_cache->set_offset = set_offset;
    new_cache->block_mask = max >> (tag_size + set_offset); 
    new_cache->set_mask = ~(new_cache->tag_mask + new_cache->block_mask);
    new_cache->block_offset = block_offset;
    new_cache->num_ways = ways;

    
    new_cache->sets = (set_t*)(malloc(sizeof(set_t) * sets));
    for(int i = 0; i < sets; i ++)
    {
        new_cache->sets[i].blocks = (block_t*)(malloc(sizeof(block_t) * ways));
        for (int j = 0; j < ways; j ++)
        {
            new_cache->sets[i].blocks[j].data = (uint32_t*)(malloc(sizeof(uint32_t) * (block/4)));
        }
    }

    return new_cache;
}

void cache_destroy(cache_t *c)
{
    int num_ints = pow_custom(c->block_offset) / 4;
    int num_sets = pow_custom(c->set_offset);

    for(int i = 0; i < num_sets; i ++) {
        for (int j = 0; j < c->num_ways; j ++) {
            free(c->sets[i].blocks[j].data);
            
        }
        free(c->sets[i].blocks);
    }
    free(c->sets);
    free(c);
}

//if hit, return 1 and update set index and way index; 
//if miss return 0 and update the set index and way index with the empty or evict vitim block;
int hit_or_miss(cache_t *c, uint64_t addr, int* set_index, int* way_index) { 
    *set_index = (addr & c->set_mask) >> c->block_offset;
    uint64_t tag = (addr & c->tag_mask) >> (c->block_offset + c->set_offset);
    printf("Set mask: %lx\n", c->set_mask);
    printf("Address: %lx\n", addr);
    printf("Set Index: %x\n", *set_index);
    printf("Tag: %lx\n", tag);
    printf("Block Offset: %x\n", c->block_offset);
    printf("Set Offset: %x\n", c->set_offset);

    //return 1 if hit here
    for(int i = 0; i < c->num_ways; i ++) {
        if (c->sets[*set_index].blocks[i].valid && c->sets[*set_index].blocks[i].tag == tag) {
            *way_index = i;
            return 1;
        }
    }
    
    // on miss, find the empty block
    for(int i = 0; i < c->num_ways; i ++) {
        if (!(c->sets[*set_index].blocks[i].valid)) {
            *way_index = i;
            return 0;
        }
    }    

    // on miss, find the LRU block
    *way_index = 0;    //initialize 
    int min = c->sets[*set_index].blocks[0].time_last_accessed;
    for(int i = 1; i < c->num_ways; i ++) {
        int new = c->sets[*set_index].blocks[i].time_last_accessed;
        if (new < min) {
            min = new;
            *way_index = i;
        }
    }   
    return 0;
}


//check hit-or-miss in the main pipe.c and stall properly if miss
// int set_index, way_index;
// int hit_or_miss = hit_or_miss(c, addr, &set_index, &way_index)
int cache_read(cache_t *c, uint64_t addr, uint64_t curr_cycle, int set_index, int way_index)
{
    uint64_t data_offset = (addr & c->block_mask) >> 2; //is this correct? 
    printf("Data offset: %lx \n", data_offset);
    uint32_t data_to_return;

    c->sets[set_index].blocks[way_index].time_last_accessed = curr_cycle;
    c->sets[set_index].last_accessed_index = way_index;
    data_to_return = c->sets[set_index].blocks[way_index].data[data_offset & (0xF)];
    return data_to_return;
}

// assume hit and already allocated if miss
// write-thru 
void cache_write(cache_t *c, uint64_t addr, uint64_t curr_cycle, int set_index, int way_index, uint32_t value) {
    uint64_t data_offset = (addr & 0x1f) >> 2;
    printf("Write Data_offset: %ld \n", data_offset);
    printf("Addr: %lx \n", addr);
    printf("Value: %d \n", value);
    c->sets[set_index].blocks[way_index].time_last_accessed = curr_cycle;
    c->sets[set_index].last_accessed_index = way_index;

    c->sets[set_index].blocks[way_index].data[data_offset & 0x7] = value;
    mem_write_32(addr, value);
}


// the update block becomes the most-recently-used block
int cache_update(cache_t *c, uint64_t addr, uint64_t curr_cycle, int set_index, int way_index) {

    uint64_t add_start = addr & (~(c->block_mask));
    for (int i = 0; i < 8; i ++) {
        c->sets[set_index].blocks[way_index].data[i] = mem_read_32(add_start);
        printf("data: %d \n", c->sets[set_index].blocks[way_index].data[i]);
        add_start += 4;
    }
    c->sets[set_index].blocks[way_index].tag = (addr & c->tag_mask) >> (c->block_offset + c->set_offset);
    c->sets[set_index].blocks[way_index].time_last_accessed = curr_cycle;
    c->sets[set_index].blocks[way_index].valid = 1;
}

