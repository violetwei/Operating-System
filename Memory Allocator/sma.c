/*
 * =====================================================================================
 *
 *	Filename:  		sma.c
 *
 *  Description:	Base code for Assignment 3 for ECSE-427 / COMP-310
 *
 *  Version:  		1.0
 *  Created:  		6/11/2020 9:30:00 AM
 *  Revised:  		-
 *  Compiler:  		gcc
 *
 *  Author:  		Mohammad Mushfiqur Rahman
 *      
 *  Instructions:   Please address all the "TODO"s in the code below and modify 
 * 					them accordingly. Feel free to modify the "PRIVATE" functions.
 * 					Don't modify the "PUBLIC" functions (except the TODO part), unless
 * 					you find a bug! Refer to the Assignment Handout for further info.
 * =====================================================================================
 */

/* Includes */
#include "sma.h" // Please add any libraries you plan to use inside this file
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/* Definitions*/
#define MAX_TOP_FREE (128 * 1024) // Max top free block size = 128 Kbytes
//	TODO: Change the Header size if required
#define FREE_BLOCK_HEADER_SIZE 2 * sizeof(char *) + sizeof(int) // Size of the Header in a free memory block
//	TODO: Add constants here
char str[60];

typedef enum //	Policy type definition
{
	WORST,
	NEXT
} Policy;

char *sma_malloc_error;
void *freeListHead = NULL;			  //	The pointer to the HEAD of the doubly linked free memory list
void *freeListTail = NULL;			  //	The pointer to the TAIL of the doubly linked free memory list
unsigned long totalAllocatedSize = 0; //	Total Allocated memory in Bytes
unsigned long totalFreeSize = 0;	  //	Total Free memory in Bytes in the free memory list
Policy currentPolicy = WORST;		  //	Current Policy
//	TODO: Add any global variables here
static sma_header_t *sma_free_list = NULL;
static void *sma_last_brk = NULL;
static bool sma_initialized = false;
static sma_header_t *search_start = NULL;
static sma_header_t *heap_start = NULL;

/*
 * =====================================================================================
 *	Public Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: sma_malloc
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates a memory block of input size from the heap, and returns a 
 * 					pointer pointing to it. Returns NULL if failed and sets a global error.
 */
void *sma_malloc(int size)
{
	if (size ==  0) return NULL;
	if (!sma_initialized && !sma_initialize()) return NULL;
	void *pMemory = NULL;

	size_t aligned_size = sma_round_up(size, SMA_DATA_ALIGN);
    if (aligned_size == 0) {
        return NULL;
    }

	//sma_header_t *header = (sma_header_t *)allocate_freeList(aligned_size);
	sma_header_t * header = sma_find_free_block(aligned_size);

	// Checks if the free list is empty
	if (header == NULL)
	{
		// Allocate memory by increasing the Program Break
		//pMemory = allocate_pBrk(aligned_size);
		header = sma_sbrk_new_block(aligned_size);
		if (header == NULL) return NULL;
	}
	// If free list is not empty
	/*else
	{
		// Allocate memory from the free memory list
		pMemory = allocate_freeList(size);

		// If a valid memory could NOT be allocated from the free memory list
		if (pMemory == (void *)-2)
		{
			// Allocate memory by increasing the Program Break
			pMemory = allocate_pBrk(size);
		}

	}*/
	sma_split_block(header, aligned_size);

    /* Remove block from the free list */
    remove_block_freeList(header);

    /* Mark block as allocated */
    sma_set_used(curr, header, 1);
    sma_set_used(prev, sma_next(header), 1);

	// Validates memory allocation
	/*if (pMemory < 0 || pMemory == NULL)
	{
		sma_malloc_error = "Error: Memory allocation failed!";
		return NULL;
	}*/

	// Updates SMA Info
	//totalAllocatedSize += size;
	totalAllocatedSize += aligned_size;

    return sma_header_to_data(header);
	//return pMemory;
}

/*
 *	Funcation Name: sma_free
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Deallocates the memory block pointed by the input pointer
 */
void sma_free(void *ptr)
{
	//	Checks if the ptr is NULL
	if (ptr == NULL)
	{
		puts("Error: Attempting to free NULL!");
	}
	//	Checks if the ptr is beyond Program Break
	else if (ptr > sbrk(0))
	{
		puts("Error: Attempting to free unallocated space!");
	}
	else
	{
		sma_header_t *header = sma_data_to_header(ptr);

		/* Mark block as free */
		sma_set_used(curr, header, 0);
		sma_set_used(prev, sma_next(header), 0);

		//	Adds the block to the free memory list
		add_block_freeList(ptr); //header

		sma_coalesce(header);
	}
}

/*
 *	Funcation Name: sma_mallopt
 *	Input type:		int
 * 	Output type:	void
 * 	Description:	Specifies the memory allocation policy
 */
void sma_mallopt(int policy)
{
	// Assigns the appropriate Policy
	if (policy == 1)
	{
		currentPolicy = WORST;
	}
	else if (policy == 2)
	{
		currentPolicy = NEXT;
	}
}

/*
 *	Funcation Name: sma_mallinfo
 *	Input type:		void
 * 	Output type:	void
 * 	Description:	Prints statistics about current memory allocation by SMA.
 */
void sma_mallinfo()
{
	//	Finds the largest Contiguous Free Space (should be the largest free block)
	int largestFreeBlock = get_largest_freeBlock();
	char str[60];

	//	Prints the SMA Stats
	sprintf(str, "Total number of bytes allocated: %lu", totalAllocatedSize);
	puts(str);
	sprintf(str, "Total free space: %lu", totalFreeSize);
	puts(str);
	sprintf(str, "Size of largest contigious free space (in bytes): %d", largestFreeBlock);
	puts(str);
}

/*
 *	Funcation Name: sma_realloc
 *	Input type:		void*, int
 * 	Output type:	void*
 * 	Description:	Reallocates memory pointed to by the input pointer by resizing the
 * 					memory block according to the input size.
 */
void *sma_realloc(void *ptr, int size)
{
	// TODO: 	Should be similar to sma_malloc, except you need to check if the pointer address
	//			had been previously allocated.
	// Hint:	Check if you need to expand or contract the memory. If new size is smaller, then
	//			chop off the current allocated memory and add to the free list. If new size is bigger
	//			then check if there is sufficient adjacent free space to expand, otherwise find a new block
	//			like sma_malloc.
	//			Should not accept a NULL pointer, and the size should be greater than 0.

	// realloc(NULL, size) is the same as malloc(size)
	if (ptr == NULL) {
		return sma_malloc(size);
	}
	// realloc(ptr, 0) is the same as free(ptr)
	if (size == 0) {
		sma_free(ptr);
		return NULL;
	}
	size_t aligned_size = sma_round_up(size, SMA_DATA_ALIGN);

    /* Find header for the user data */
    sma_header_t *header = sma_data_to_header(ptr);

    /* Save original size of block */
    size_t orig_size = sma_size(curr, header);

    /*
     * If we're shrinking the block, try to split it
     * and return the same block. No copying required.
     */
    if (aligned_size <= orig_size) {
        sma_split_block(header, aligned_size);
        return ptr;
	}

    /* Try coalescing with the next block */
    if (sma_coalesce_next(header)) {
        if (aligned_size <= sma_size(curr, header)) {
            sma_split_block(header, aligned_size);
            return ptr;
        }
    }

    /* If this is the last block, just sbrk some more memory */
    if (sma_is_sentinel(curr, sma_next(header))) {
        size_t sbrk_size = aligned_size - sma_size(curr, header);
        sma_header_t *next_alloc = sma_sbrk_new_block(sbrk_size);
        if (next_alloc != NULL) {
            sma_coalesce_next(header);
            sma_split_block(header, aligned_size);
            return ptr;
        }
    }

    /*
     * We tried everything but we still can't resize it in-place,
     * fall back to malloc() followed by memcpy().
     */
    void *new_ptr = sma_malloc(size);
    if (new_ptr != NULL) {
        memcpy(new_ptr, ptr, orig_size);
        sma_free(ptr);
    }
    return new_ptr;
}

/*
 * =====================================================================================
 *	Private Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: allocate_pBrk
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory by increasing the Program Break
 */
void *allocate_pBrk(int size)
{
	void *newBlock = NULL;
	int excessSize;

	//	TODO: 	Allocate memory by incrementing the Program Break by calling sbrk() or brk()
	//	Hint:	Getting an exact "size" of memory might not be the best idea. Why?
	//			Also, if you are getting a larger memory, you need to put the excess in the free list
	//newBlock = sbrk(size);
	size_t page_size = sma_round_up(size + SMA_INFO_SIZE, SMA_SBRK_ALIGN);

    /* Request some more memory from the kernel */
    void *orig_brk, *new_brk;
    if (!sma_sbrk(page_size, &orig_brk, &new_brk)) {
        return NULL;
    }

    /* New block starts right before the data break */
    sma_header_t *header = sma_data_to_header(orig_brk);

    /* Convert sentinel to a normal block */
    sma_set_size(curr, header, page_size - SMA_INFO_SIZE);
    sma_set_used(curr, header, 0);

    /* Initialize new sentinel block */
    sma_header_t *sentinel = sma_data_to_header(new_brk);
    sma_set_size(prev, sentinel, page_size - SMA_INFO_SIZE);
    sma_set_used(prev, sentinel, 0);
    sma_set_size(curr, sentinel, 0);
    sma_set_used(curr, sentinel, 1);

    /* Add new block to free list */
    add_block_freeList(header);

    return sma_coalesce_prev(header);

	//	Allocates the Memory Block
	//allocate_block(newBlock, size, excessSize, 0);
    // return newBlock;
}

static sma_header_t * sma_sbrk_new_block(size_t aligned_size) {
    /* Round to a multiple of the page size */
    size_t page_size = sma_round_up(aligned_size + SMA_INFO_SIZE, SMA_SBRK_ALIGN);

    /* Request some more memory from the kernel */
    void *orig_brk, *new_brk;
    if (!sma_sbrk(page_size, &orig_brk, &new_brk)) {
        return NULL;
    }

    /* New block starts right before the data break */
    sma_header_t *header = sma_data_to_header(orig_brk);

    /* Convert sentinel to a normal block */
    sma_set_size(curr, header, page_size - SMA_INFO_SIZE);
    sma_set_used(curr, header, 0);

    /* Initialize new sentinel block */
    sma_header_t *sentinel = sma_data_to_header(new_brk);
    sma_set_size(prev, sentinel, page_size - SMA_INFO_SIZE);
    sma_set_used(prev, sentinel, 0);
    sma_set_size(curr, sentinel, 0);
    sma_set_used(curr, sentinel, 1);

    /* Add new block to free list */
    add_block_freeList(header);

	totalFreeSize += aligned_size;

    /* Coalesce with the previous block if it's free */
    return sma_coalesce_prev(header);
}

static sma_header_t * sma_find_free_block(size_t aligned_size) {
	if (currentPolicy == WORST) {
		sma_header_t *header = sma_free_list;
		while (header != NULL) {
			if (sma_size(curr, header) >= aligned_size) {
				return header;
			}
			header = header->next_free;
		}
	} else { // Next-fit
		if (!search_start) {
			search_start = sma_free_list;
		}
		sma_header_t *header = search_start;
		while (header != NULL) {
			if (sma_size(curr, header) >= aligned_size) {
				search_start = header;
				return header;
			}
			header = header->next_free;
		}
	}
    return NULL;
}

/*
 *	Funcation Name: allocate_freeList
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory from the free memory list
 */
void *allocate_freeList(int size)
{
	void *pMemory = NULL;

	if (currentPolicy == WORST)
	{
		// Allocates memory using Worst Fit Policy
		pMemory = allocate_worst_fit(size);
	}
	else if (currentPolicy == NEXT)
	{
		// Allocates memory using Next Fit Policy
		pMemory = allocate_next_fit(size);
	}
	else
	{
		pMemory = NULL;
	}

	return pMemory;
}

/*
 *	Funcation Name: allocate_worst_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Worst Fit from the free memory list
 */
void *allocate_worst_fit(int size)
{
	void *worstBlock = NULL;
	int excessSize;
	int blockFound = 0;

	//	TODO: 	Allocate memory by using Worst Fit Policy
	//	Hint:	Start off with the freeListHead and iterate through the entire list to 
	//			get the largest block
	sma_header_t *header = sma_free_list;
    while (header != NULL) {
        if (sma_size(curr, header) >= size) {
            return header;
        }
        header = header->next_free;
    }

	//	Checks if appropriate block is found.
	if (blockFound)
	{
		//	Allocates the Memory Block
		allocate_block(worstBlock, size, excessSize, 1);
	}
	else
	{
		//	Assigns invalid address if appropriate block not found in free list
		worstBlock = (void *)-2;
	}

	return worstBlock;
}

/*
 *	Funcation Name: allocate_next_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Next Fit from the free memory list
 */
void *allocate_next_fit(int size)
{
	void *nextBlock = NULL;
	int excessSize;
	int blockFound = 0;

	//	TODO: 	Allocate memory by using Next Fit Policy
	//	Hint:	You should use a global pointer to keep track of your last allocated memory address, and 
	//			allocate free blocks that come after that address (i.e. on top of it). Once you reach 
	//			Program Break, you start from the beginning of your heap, as in with the free block with
	//			the smallest address)
	if (!search_start) {
		search_start = sma_free_list;
	}
	sma_header_t *header = search_start;
    while (header != NULL) {
        if (sma_size(curr, header) >= size) {
			search_start = header;
            return header;
        }
        header = header->next_free;
    }

	//	Checks if appropriate found is found.
	if (blockFound)
	{
		//	Allocates the Memory Block
		allocate_block(nextBlock, size, excessSize, 1);
	}
	else
	{
		//	Assigns invalid address if appropriate block not found in free list
		nextBlock = (void *)-2;
	}

	return nextBlock;
}

static bool sma_initialize() {
    /* Initialize cached brk value */
    sma_last_brk = sbrk(0);
    if (sma_last_brk == (void *)-1) {
        return false;
    }

    /* Allocate some starting memory */
    void *orig_brk, *new_brk;
    if (!sma_sbrk(SMA_SBRK_ALIGN, &orig_brk, &new_brk)) {
        return false;
    }

    /* Set up the sentinel blocks */
    sma_header_t *bottom = sma_as_header(orig_brk);
    sma_set_size(prev, bottom, 0);
    sma_set_used(prev, bottom, 1);

    sma_header_t *top = sma_data_to_header(new_brk);
    sma_set_size(curr, top, 0);
    sma_set_used(curr, top, 1);

    /* Initialize the initial free block */
    sma_set_size(curr, bottom, SMA_SBRK_ALIGN - 2 * SMA_INFO_SIZE);
    sma_set_used(curr, bottom, 0);

    sma_set_size(prev, top, SMA_SBRK_ALIGN - 2 * SMA_INFO_SIZE);
    sma_set_used(prev, top, 0);

    /* Add free block to free list */
    add_block_freeList(bottom);

    sma_initialized = true;
    return true;
}

static sma_header_t * sma_split_block(sma_header_t *header, size_t aligned_size) {
    size_t curr_size = sma_size(curr, header);

    /* Only split if we have enough space for another allocation */
    if (curr_size < aligned_size + SMA_INFO_SIZE + SMA_DATA_ALIGN) {
        return NULL;
    }

    /* This will be the size of our split block */
    size_t split_size = curr_size - aligned_size - SMA_INFO_SIZE;
    
    /* Update the previous field of the next adjacent block */
    sma_header_t *next_header = sma_next(header);
    sma_set_size(prev, next_header, split_size);
    sma_set_used(prev, next_header, 0);

    /* Update the original header with the new size */
    sma_set_size(curr, header, aligned_size);

    /* Now find out where our split header is */
    sma_header_t *split_header = sma_next(header);

    /* Initialize prev field of split header */
    sma_set_size(prev, split_header, aligned_size);
    sma_set_used(prev, split_header, sma_used(curr, header));

    /* Initialize curr field of split header */
    sma_set_size(curr, split_header, split_size);
    sma_set_used(curr, split_header, 0);

    /* Add new block to free list */
    add_block_freeList(split_header);

    /* Try to coalesce new block with the next adjacent block */
    sma_coalesce_next(split_header);

    return split_header;
}

/*
 *	Funcation Name: allocate_block
 *	Input type:		void*, int, int, int
 * 	Output type:	void
 * 	Description:	Performs routine operations for allocating a memory block
 */
void allocate_block(void *newBlock, int size, int excessSize, int fromFreeList)
{
	void *excessFreeBlock; //	pointer for any excess free block
	int addFreeBlock;

	// 	Checks if excess free size is big enough to be added to the free memory list
	//	Helps to reduce external fragmentation

	//	TODO: Adjust the condition based on your Head and Tail size (depends on your TAG system)
	//	Hint: Might want to have a minimum size greater than the Head/Tail sizes
	addFreeBlock = excessSize > FREE_BLOCK_HEADER_SIZE;

	//	If excess free size is big enough
	if (addFreeBlock)
	{
		//	TODO: Create a free block using the excess memory size, then assign it to the Excess Free Block

		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//	Removes new block and adds the excess free block to the free list
			replace_block_freeList(newBlock, excessFreeBlock);
		}
		else
		{
			//	Adds excess free block to the free list
			add_block_freeList(excessFreeBlock);
		}
	}
	//	Otherwise add the excess memory to the new block
	else
	{
		//	TODO: Add excessSize to size and assign it to the new Block

		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//	Removes the new block from the free list
			remove_block_freeList(newBlock);
		}
	}
}

static sma_header_t *sma_coalesce(sma_header_t *header) {
    /* Coalesce forwards */
    sma_coalesce_next(header);

    /* Coalesce backwards */
    return sma_coalesce_prev(header);
}

static bool sma_coalesce_next(sma_header_t *header) {
    /* Can't coalesce if the next adjacent block is allocated */
    sma_header_t *next_adj = sma_next(header);
    if (sma_used(curr, next_adj)) {
        return false;
    }

    remove_block_freeList(next_adj);

    size_t new_size = sma_size(curr, header) + SMA_INFO_SIZE + sma_size(curr, next_adj);

    sma_header_t *next_next_adj = sma_next(next_adj);
    sma_set_used(prev, next_next_adj, sma_used(curr, header));
    sma_set_size(prev, next_next_adj, new_size);

    sma_set_size(curr, header, new_size);

    return true;
}

static sma_header_t * sma_coalesce_prev(sma_header_t *header) {
    if (!sma_used(prev, header)) {
        header = sma_prev(header);
        sma_coalesce_next(header);
    }
    return header;
}

static bool sma_sbrk(size_t delta, void **orig_brk, void **new_brk) {
    /* If allocation would overflow, fail fast */
    if ((size_t)sma_last_brk + delta < (size_t)sma_last_brk) {
        return false;
    }

    /* We know the allocation is safe, call sbrk */
    void *last_brk;
    if ((last_brk = sbrk(delta)) == (void *)-1) {
        return false;
    }

    /* Update cached brk value */
    sma_last_brk = (void *)((char *)last_brk + delta);

    *orig_brk = last_brk;
    *new_brk = sma_last_brk;
    return true;
}


/*
 *	Funcation Name: replace_block_freeList
 *	Input type:		void*, void*
 * 	Output type:	void
 * 	Description:	Replaces old block with the new block in the free list
 */
void replace_block_freeList(void *oldBlock, void *newBlock)
{
	//	TODO: Replace the old block with the new block
	//	Removes new block and adds the excess free block to the free list
	remove_block_freeList(oldBlock);
	add_block_freeList(newBlock);

	//	Updates SMA info
	totalAllocatedSize += (get_blockSize(oldBlock) - get_blockSize(newBlock));
	totalFreeSize += (get_blockSize(newBlock) - get_blockSize(oldBlock));
}

/*
 *	Funcation Name: add_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Adds a memory block to the the free memory list
 */
void add_block_freeList(void *block)
{
	//	TODO: 	Add the block to the free list
	//	Hint: 	You could add the free block at the end of the list, but need to check if there
	//			exits a list. You need to add the TAG to the list.
	//			Also, you would need to check if merging with the "adjacent" blocks is possible or not.
	//			Merging would be tideous. Check adjacent blocks, then also check if the merged
	//			block is at the top and is bigger than the largest free block allowed (128kB).
	sma_header_t *header = (sma_header_t *)block;
	header->prev_free = NULL;
    header->next_free = sma_free_list;
    if (sma_free_list != NULL) {
        sma_free_list->prev_free = header;
    }
    sma_free_list = header;

	//	Updates SMA info
	totalAllocatedSize -= get_blockSize(header);
	totalFreeSize += get_blockSize(header);
}

/*
 *	Funcation Name: remove_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Removes a memory block from the the free memory list
 */
void remove_block_freeList(void *block)
{
	//	TODO: 	Remove the block from the free list
	//	Hint: 	You need to update the pointers in the free blocks before and after this block.
	//			You also need to remove any TAG in the free block.
	sma_header_t *header = (sma_header_t *)block;
	sma_header_t *prev_free = header->prev_free;
    sma_header_t *next_free = header->next_free;

    if (sma_free_list == header) {
        sma_free_list = next_free;
    }

    if (prev_free != NULL) {
        prev_free->next_free = next_free;
    }

    if (next_free != NULL) {
        next_free->prev_free = prev_free;
    }

	//	Updates SMA info
	totalAllocatedSize += get_blockSize(header);
	totalFreeSize -= get_blockSize(header);
}

/*
 *	Funcation Name: get_blockSize
 *	Input type:		void*
 * 	Output type:	int
 * 	Description:	Extracts the Block Size
 */
int get_blockSize(void *ptr)
{
	int *pSize;

	//	Points to the address where the Length of the block is stored
	pSize = (int *)ptr;
	pSize--;

	//	Returns the deferenced size
	return *(int *)pSize;
}

/*
 *	Funcation Name: get_largest_freeBlock
 *	Input type:		void
 * 	Output type:	int
 * 	Description:	Extracts the largest Block Size
 */
int get_largest_freeBlock()
{
	int largestBlockSize = 0;

	//	TODO: Iterate through the Free Block List to find the largest free block and return its size
	sma_header_t *header = sma_free_list;
	while (header != NULL) {
		if (sma_size(curr, header) >= largestBlockSize) {
			largestBlockSize = sma_size(curr, header);
		}
		header = header->next_free;
	}

	return largestBlockSize;
}
