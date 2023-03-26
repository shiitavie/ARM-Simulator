/*
 * CMSC 22200, Fall 2021
 *
 * ARM pipeline timing simulator
 *
 */
#ifndef _CACHE_H_
#define _CACHE_H_

#include <stdint.h>

typedef struct block_t
{
    uint64_t tag;
    char valid;
    uint32_t* data; //32-byte data = 8 * uint32_t
    uint64_t time_last_accessed;
} block_t;

typedef struct set_t
{
    block_t* blocks;
    int last_accessed_index;
} set_t;

typedef struct cache_t
{
    set_t* sets;
    uint64_t set_mask;
    uint64_t block_mask;
    uint64_t tag_mask;
    int num_ways;
    int num_sets;
    int set_offset;
    int block_offset;
    //int num_bytes; //dont need this? 
} cache_t;

cache_t *cache_new(int sets, int ways, int block);
void cache_destroy(cache_t *c);
int hit_or_miss(cache_t *c, uint64_t addr, int* set_index, int* way_index);
int cache_update(cache_t *c, uint64_t addr, uint64_t curr_cycle, int set_index, int way_index);
int cache_read(cache_t *c, uint64_t addr, uint64_t curr_cycle, int set_index, int way_index);
void cache_write(cache_t *c, uint64_t addr, uint64_t curr_cycle, int set_index, int way_index, uint32_t value);

#endif
