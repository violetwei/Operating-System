/*
 * =====================================================================================
 *
 *  Filename:  		sma.h
 *
 *  Description:	Header file for SMA.
 *
 *  Version:  		1.0
 *  Created:  		3/11/2020 9:30:00 AM
 *  Revised:  		-
 *  Compiler:  		gcc
 *
 *  Author:  		Mohammad Mushfiqur Rahman
 *      
 *  Instructions:   Please address all the "TODO"s in the code below and modify them
 *                  accordingly. Refer to the Assignment Handout for further info.
 * =====================================================================================
 */

/* Includes */
//  TODO: Add any libraries you might use here.
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//  Policies definition
#define WORST_FIT	1
#define NEXT_FIT	2

#define SMA_INFO_SIZE (2 * sizeof(size_t))
#define SMA_DATA_ALIGN (2 * sizeof(size_t))
#define SMA_SBRK_ALIGN 4096
#define SMA_MASK_SIZE ~0x7
#define SMA_FLAG_USED 0x1
#define sma_round_up(x, align) (((x) + (align) - 1) & -(align))
#define sma_as_bytes(x) ((char *)(x))
#define sma_as_header(x) ((sma_header_t *)(x))
#define sma_data_to_header(d) (sma_as_header(sma_as_bytes(d) - SMA_INFO_SIZE))
#define sma_header_to_data(h) (sma_as_bytes(h) + SMA_INFO_SIZE)
#define sma_info(w, h) (h->w##_info)
#define sma_size(w, h) (sma_info(w, h) & SMA_MASK_SIZE)
#define sma_used(w, h) (sma_info(w, h) & SMA_FLAG_USED)
#define sma_set_size(w, h, v) (sma_info(w, h) = (sma_info(w, h) & ~SMA_MASK_SIZE) | (v))
#define sma_set_used(w, h, v) (sma_info(w, h) = (sma_info(w, h) & ~SMA_FLAG_USED) | (v))
#define sma_is_sentinel(w, h) (sma_size(w, h) == 0)
#define sma_next(h) (sma_as_header(sma_header_to_data(h) + sma_size(curr, h)))
#define sma_prev(h) (sma_data_to_header(sma_as_bytes(h) - sma_size(prev, h)))

extern char *sma_malloc_error;

typedef struct sma_header {
    // Holds the user data size in bytes and status flags of the previous adjacent block.
    size_t prev_info;

    // Holds the user data size in bytes and status flags of the current block.
    size_t curr_info;

    // If the current block is not in use, holds a pointer to the previous block in the free list
    struct sma_header *prev_free;

    // If the current block is not in use, holds a pointer to the next block in the free list.
    struct sma_header *next_free;
} sma_header_t;


//  Public Functions declaration
void *sma_malloc(int size);
void sma_free(void* ptr);
void sma_mallopt(int policy);
void sma_mallinfo();
void *sma_realloc(void *ptr, int size);

//  Private Functions declaration
static void* allocate_pBrk(int size);
static void* allocate_freeList(int size);
static void* allocate_worst_fit(int size);
static void* allocate_next_fit(int size);
static void allocate_block(void* newBlock, int size, int excessSize, int fromFreeList);
static void replace_block_freeList(void* oldBlock, void* newBlock);
static void add_block_freeList(void* block);
static void remove_block_freeList(void* block);
static int get_blockSize(void *ptr);
static int get_largest_freeBlock();
//  TODO: Declare any private functions that you intend to add in your code.
static sma_header_t * sma_coalesce_prev(sma_header_t *header);
static bool sma_coalesce_next(sma_header_t *header);
static sma_header_t * sma_coalesce(sma_header_t *header);
static bool sma_initialize(void);
static bool sma_sbrk(size_t delta, void **orig_brk, void **new_brk);
static sma_header_t * sma_split_block(sma_header_t *header, size_t aligned_size);
static sma_header_t * sma_find_free_block(size_t aligned_size);
static sma_header_t * sma_sbrk_new_block(size_t aligned_size);
