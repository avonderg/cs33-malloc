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
block_t* coalesce(void *b);

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
prologue = mem_sbrk(TAGS_SIZE);
if (prologue == (void *)-1) {
    return -1;
}
epilogue = mem_sbrk(TAGS_SIZE);
if (epilogue == (void *)-1) {
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
    new = mem_sbrk(size);
    if (new == (void *)-1) { // ask for more memory (can't find a fit)
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
    block = coalesce(ptr); // coalesce
    insert_free_block(block);
}

// write def
block_t* coalesce(void *b) {
    block_t *t = payload_to_block(b);
    block_t *next = block_next(t);
    block_t *prev = block_prev(t);
    if (!(block_prev_allocated(t)) && !(block_next_allocated(t))) { // if next and prev are unallocated (free)
       // add up blocks
       // put in new block of that size into the free list
        size_t one = block_size(next);
        size_t two = block_size(prev);
        size_t three = block_size(t);
        assert(!block_allocated(next));
        pull_free_block(next);
        block_set_allocated(next, 0);
        pull_free_block(prev);
        block_set_allocated(prev, 0);
        block_set_allocated(t, 0);
        block_set_size(prev, (one+two+three));
       // set pointer to prev
        t = prev;
    }
    else if ((block_prev_allocated(t)) && !(block_next_allocated(t))) { // if prev allocated, next unallocated
        size_t one = block_size(next);
        size_t three = block_size(t);
        pull_free_block(next);
        block_set_allocated(next, 0);
        block_set_allocated(t, 0);
        block_set_size(t, (one+three));
    }
    else if (!(block_prev_allocated(t)) && (block_next_allocated(t))) { // if prev unallocated, next allocated
        size_t two = block_size(prev);
        size_t three = block_size(t);
        pull_free_block(prev);
        block_set_allocated(prev, 0);
        block_set_allocated(t, 0);
        block_set_size(prev, (two+three));
       // set pointer to prev
        t = prev;
    }
    else {
        return t;
    }
    return t;
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
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    size_t oldsize = size;
    size = align(size) + TAGS_SIZE; // where do i align the size
    if (ptr == NULL) {
        mm_malloc(oldsize);
    }
    block_t *block = payload_to_block(ptr);
    size_t original = block_size(block);
    size_t requested = size;
    if (((original - requested) >= 16* MINBLOCKSIZE) && (requested <= (original / 2))) { // splitting if requested size smaller than ptr
        block_set_size(block, requested);
        block_t *freed = block_next(block);
        block_set_size_and_allocated(freed, original-requested, 0);
        insert_free_block(freed);
        return ptr;
    }
    size_t to_check = original + block_size(block_next(block));
    size_t to_check2 = original + block_size(block_prev(block));
     if (!block_prev_allocated(block) && (requested <= to_check2)) { // if next block is unallocated and original + next big enough
        block_t *freed = block_prev(block);
        pull_free_block(freed);
        if ((to_check2 - requested) >= 16 *MINBLOCKSIZE && (requested <= (to_check2 / 2))) {
        block_set_size(block, requested);
        block_set_size_and_allocated(block_prev(block), to_check2-requested, 0); 
        insert_free_block(block_prev(block));
        return ptr;
        }
        block_set_size(block, to_check2);
        return ptr;
    }
    if (!block_next_allocated(block) && (requested <= to_check)) { // if next block is unallocated and original + next big enough
        block_t *freed = block_next(block);
        pull_free_block(freed);
        if ((to_check - requested) >= 16 *MINBLOCKSIZE && (requested <= (to_check / 2))) {
        block_set_size(block, requested);
        block_set_size_and_allocated(block_next(block), to_check-requested, 0); 
        insert_free_block(block_next(block));
        return ptr;
        }
        block_set_size(block, to_check);
        return ptr;
    }
    else { // otherwise, searches free list
        void *ret = mm_malloc(requested); // get large enough block
        if (ret == NULL) {
            fprintf(stderr, "malloc");
        }
        if (requested < original) {
            void *to_return = memcpy(ret, ptr, requested);
            mm_free(ptr);
            return to_return;
        }
        void *to_return = memcpy(ret, ptr, original);
        mm_free(ptr);
        return to_return;
    }
    
    // if requested is bigger:
    // 1. next is free -> check if size of original+next big enough to hold -> extend original
    // 2. look through free list to find free block big enough -> call malloc (gives block big enough to hold, but input has certain
    // amoint being used, since moving input -> copy over memory that first bytes to new block before returning
    // so memory is unchaged

    // nxt case -> if you can try to expand into next block (coalesce), if possible,
    // try to copy over
    // if expand to left (use memcpy)
}
