#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * BEFORE GETTING STARTED:
 *
 * Familiarize yourself with the functions and constants/variables
 * in the following included files.
 * This will make the project a LOT easier as you go!!
 *
 * The diagram in Section 4.1 (Specification) of the handout will help you
 * understand the constants in mm.h
 * Section 4.2 (Support Routines) of the handout has information about
 * the functions in mminline.h and memlib.h
 */
#include "./memlib.h"
#include "./mm.h"
#include "./mminline.h"
block_t* prologue;
block_t* epilogue;
void coalesce(void *b);

// rounds up to the nearest multiple of WORD_SIZE
static inline size_t align(size_t size) {
    return (((size) + (WORD_SIZE - 1)) & ~(WORD_SIZE - 1));
}

/*
 *                             _       _ _
 *     _ __ ___  _ __ ___     (_)_ __ (_) |_
 *    | '_ ` _ \| '_ ` _ \    | | '_ \| | __|
 *    | | | | | | | | | | |   | | | | | | |_
 *    |_| |_| |_|_| |_| |_|___|_|_| |_|_|\__|
 *                       |_____|
 *
 * initializes the dynamic storage allocator (allocate initial heap space)
 * arguments: none
 * returns: 0, if successful
 *         -1, if an error occurs
 */
int mm_init(void) {
// init flist_first
// look at gearup and handout 
// flist_first = NULL;
// allocate prologue and epilogue
// allocate initial heap area
if ((prologue = mem_sbrk(TAGS_SIZE)) == -1) {
    return -1;
}
if ((epilogue = mem_sbrk(TAGS_SIZE)) == -1) {
    return -1;
}
flist_first = NULL; // not null if there is a free block
block_set_size_and_allocated(prologue, TAGS_SIZE, 1);
block_set_size_and_allocated(epilogue, TAGS_SIZE, 1);
return 0;
}

/*     _ __ ___  _ __ ___      _ __ ___   __ _| | | ___   ___
 *    | '_ ` _ \| '_ ` _ \    | '_ ` _ \ / _` | | |/ _ \ / __|
 *    | | | | | | | | | | |   | | | | | | (_| | | | (_) | (__
 *    |_| |_| |_|_| |_| |_|___|_| |_| |_|\__,_|_|_|\___/ \___|
 *                       |_____|
 *
 * allocates a block of memory and returns a pointer to that block's payload
 * arguments: size: the desired payload size for the block
 * returns: a pointer to the newly-allocated block's payload (whose size
 *          is a multiple of ALIGNMENT), or NULL if an error occurred
 */
void *mm_malloc(size_t size) {
    // search through flist
    block_t *curr = flist_first;
    block_t *new = NULL;
    size = align(size) + TAGS_SIZE;
    if (size == 0) {
        return NULL;
    }
    while (curr != NULL) {
    if (block_size(curr) >= size) {
        block_t *alloc = curr;
        pull_free_block(curr);
        if (size > MINBLOCKSIZE && block_size(curr) - size > MINBLOCKSIZE) {
            size_t total = block_size(curr);
            block_set_size_and_allocated(alloc, size, 1);
            block_t *freed = block_next(curr);
            block_set_size_and_allocated(freed, total-size, 0); //splitting- taking total size - size allocated
            insert_free_block(freed);
        }
        block_set_allocated(curr, 1);
        return curr->payload;
    }
    curr = block_flink(curr);
    if (curr == flist_first) {
        break;
    }
    }
    if ((new = mem_sbrk(size)) == -1) { // ask for more memory (can't find a fit)
        return NULL;
    }
    new = epilogue; // new is the end
    block_set_size_and_allocated(new, size, 1);
    epilogue = block_next(new); // epilogue after new
    block_set_size_and_allocated(epilogue, TAGS_SIZE, 1);
    return new->payload;
}

/*                              __
 *     _ __ ___  _ __ ___      / _|_ __ ___  ___
 *    | '_ ` _ \| '_ ` _ \    | |_| '__/ _ \/ _ \
 *    | | | | | | | | | | |   |  _| | |  __/  __/
 *    |_| |_| |_|_| |_| |_|___|_| |_|  \___|\___|
 *                       |_____|
 *
 * frees a block of memory, enabling it to be reused later
 * arguments: ptr: pointer to the block's payload
 * returns: nothing
 */
void mm_free(void *ptr) {
    block_t *block = payload_to_block(ptr);
    block_set_allocated(block, 0);
    coalesce(ptr); // coalesce
    insert_free_block(block);
}

// write def
void coalesce(void *b) {
    block_t *t = payload_to_block(b);
    block_t *next = block_next(t);
    block_t *prev = block_prev(t);
    if (!(block_prev_allocated(t)) && !(block_next_allocated(t))) { // if next and prev are unallocated (free)
       // add up blocks
       // put in new block of that size into the free list
        size_t one = block_size(next);
        size_t two = block_size(prev);
        size_t three = block_size(t);
        pull_free_block(next);
        block_set_allocated(next, 0);
        pull_free_block(prev);
        block_set_allocated(prev, 0);
        block_set_allocated(t, 0);
        pull_free_block(t);
        block_set_size(t, (one+two+three));
       // set pointer to prev
        t = prev;
    }
    else if ((block_prev_allocated(t)) && !(block_next_allocated(t))) { // if prev allocated, next unallocated
        size_t one = block_size(next);
        size_t three = block_size(t);
        pull_free_block(next);
        block_set_allocated(next, 0);
        block_set_allocated(t, 0);
        pull_free_block(t);
        block_set_size(t, (one+three));
    }
    else if (!(block_prev_allocated(t)) && (block_next_allocated(t))) { // if prev unallocated, next allocated
        size_t two = block_size(prev);
        size_t three = block_size(t);
        pull_free_block(prev);
        block_set_allocated(prev, 0);
        block_set_allocated(t, 0);
        pull_free_block(t);
        block_set_size(t, (two+three));
       // set pointer to prev
        t = prev;
    }
    else {
        return;
    }
}

/*
 *                                            _ _
 *     _ __ ___  _ __ ___      _ __ ___  __ _| | | ___   ___
 *    | '_ ` _ \| '_ ` _ \    | '__/ _ \/ _` | | |/ _ \ / __|
 *    | | | | | | | | | | |   | | |  __/ (_| | | | (_) | (__
 *    |_| |_| |_|_| |_| |_|___|_|  \___|\__,_|_|_|\___/ \___|
 *                       |_____|
 *
 * reallocates a memory block to update it with a new given size
 * arguments: ptr: a pointer to the memory block's payload
 *            size: the desired new payload size
 * returns: a pointer to the new memory block's payload
 */
void *mm_realloc(void *ptr, size_t size) {
    // TODO
    return NULL;
}
