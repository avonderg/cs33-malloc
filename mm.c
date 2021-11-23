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
block_t *prologue;
block_t *epilogue;
block_t *coalesce(void *b);

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
    prologue = mem_sbrk(TAGS_SIZE);  // allocates prologue
    if (prologue == (void *)-1) {    // error checking
        return -1;
    }
    epilogue = mem_sbrk(TAGS_SIZE);  // allocates epilogue
    if (epilogue == (void *)-1) {    // error checking
        return -1;
    }
    flist_first = NULL;  // since no other free blocks exist
    block_set_size_and_allocated(prologue, TAGS_SIZE, 1);  // sets size
    block_set_size_and_allocated(epilogue, TAGS_SIZE, 1);  // sets size
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
    block_t *curr = flist_first;
    block_t *new = NULL;
    size = align(size) + TAGS_SIZE;  // aligns size
    if (size == 0) {
        return NULL;
    }
    while (curr != NULL) {  // search through free list
        if (block_size(curr) >=
            size) {  // if the size is large enough to malloc
            block_t *alloc = curr;
            pull_free_block(curr);  // pulls free block
            if (size > MINBLOCKSIZE &&
                block_size(curr) - size >
                    (16*MINBLOCKSIZE)) {  // condition to check for splitting
                size_t total = block_size(curr);
                block_set_size_and_allocated(alloc, size, 1);
                block_t *freed = block_next(curr);
                block_set_size_and_allocated(
                    freed, total - size,
                    0);  // splitting- taking (total size - size allocated)
                insert_free_block(freed);  // inserts into free list
            }
            block_set_allocated(curr, 1);  // sets curr as allocated
            return curr->payload;          // returns its payload
        }
        curr = block_flink(curr);   // gets next element
        if (curr == flist_first) {  // since the list is circular -- if the head
                                    // is reached again
            break;
        }
    }
    new = mem_sbrk(size);     // otherwise, if there is no memory, asks for more
                              // (can't find a fit)
    if (new == (void *)-1) {  // error checking
        return NULL;
    }
    new = epilogue;  // the new block is the end of heap
    block_set_size_and_allocated(new, size, 1);
    epilogue = block_next(new);  // sets epilogue to be the block after new
    block_set_size_and_allocated(epilogue, TAGS_SIZE,
                                 1);  // sets epilogue to be allocated
    return new->payload;              // returns payload
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
    block_set_allocated(block, 0);  // sets block to be unallocated
    block = coalesce(ptr);          // coalesce
    insert_free_block(block);       // inserts into free list
}

/**
 * Helper function for mm_free(), coalesces free blocks surrounding a newly
 * freed block
 *
 * Parameters:
 * - b: a pointer to the block's payload
 *
 * Returns:
 * - a pointer to the block
 * **/
block_t *coalesce(void *b) {
    block_t *t = payload_to_block(b);
    block_t *next = block_next(t);  // gets next block
    block_t *prev = block_prev(t);  // gets prev block
    if (!(block_prev_allocated(t)) &&
        !(block_next_allocated(
            t))) {  // if next and prev are unallocated (free)
        // then, add up both blocks, and puts in new block of that size into the
        // free list gets sizes of all three blocks, including the current one
        size_t one = block_size(next);
        size_t two = block_size(prev);
        size_t three = block_size(t);
        assert(!block_allocated(next));
        // pulls blocks and sets them to be unallocated
        pull_free_block(next);
        block_set_allocated(next, 0);
        pull_free_block(prev);
        block_set_allocated(prev, 0);
        block_set_allocated(t, 0);
        block_set_size(prev,
                       (one + two + three));  // changes size of prev block (so
                                              // it is all one large free block)
                                              // set pointer to prev
        t = prev;
    } else if ((block_prev_allocated(t)) &&
               !(block_next_allocated(
                   t))) {  // if prev allocated, next unallocated
                           // gets sizes
        size_t one = block_size(next);
        size_t three = block_size(t);
        // pulls blocks and sets as unallocated
        pull_free_block(next);
        block_set_allocated(next, 0);
        block_set_allocated(t, 0);
        block_set_size(t, (one + three));
    } else if (!(block_prev_allocated(t)) &&
               (block_next_allocated(
                   t))) {  // if prev unallocated, next allocated
                           // gets sizes
        size_t two = block_size(prev);
        size_t three = block_size(t);
        // pulls blocks and sets as unallocated
        pull_free_block(prev);
        block_set_allocated(prev, 0);
        block_set_allocated(t, 0);
        // changes size of prev to include the current block
        block_set_size(prev, (two + three));
        // set pointer to prev
        t = prev;
    } else {
        return t;  // if there is no need to coalesce
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
    if (size == 0) {  // if size is zero, calls mm_free()
        mm_free(ptr);
        return NULL;
    }
    size_t oldsize = size;  // stors unaligned size
    size = align(size) + TAGS_SIZE;
    if (ptr == NULL) {  // if ptr is null, calls malloc
        mm_malloc(oldsize);
    }
    block_t *block = payload_to_block(ptr);
    size_t original = block_size(block);
    size_t requested = size;
    if (((original - requested) >= MINBLOCKSIZE) &&
        (requested <=
         (original / 2))) {  // splits if requested size smaller than ptr's size
        block_set_size(block, requested);
        block_t *freed = block_next(block);
        block_set_size_and_allocated(
            freed, original - requested,
            0);  // splitting- taking (original size - requested size)
        insert_free_block(freed);  // inserts this new block into free list
        return ptr;
    }
    size_t to_check =
        original +
        block_size(block_next(
            block));  // checks if the size of the next block is large enough
    if (!block_next_allocated(block) &&
        (requested <= to_check)) {  // if next block is unallocated and original
                                    // + next big enough
        block_t *freed = block_next(block);
        pull_free_block(freed);  // pulls next block from free list
        if ((to_check - requested) >= MINBLOCKSIZE &&
            (requested <= (to_check / 2))) {  // if splitting is necessary
            block_set_size(block, requested);
            block_set_size_and_allocated(
                block_next(block), to_check - requested,
                0);  // splitting- taking (combined size - requested size)
            insert_free_block(
                block_next(block));  // inserts next block into free list
            return ptr;
        }
        block_set_size(block, to_check);  // otherwise, if splitting unecessary
        return ptr;
    } else {  // otherwise, searches free list for available memory
        char to_save[requested];  // creates a buffer to save the ptr, so that
                                  // it can be freed
        memcpy(to_save, ptr, requested);  // copies over the memory
        mm_free(ptr);                     // frees current block
        void *ret = mm_malloc(
            requested);     // get large enough block for requested memory
        if (ret == NULL) {  // error checking
            fprintf(stderr, "malloc");
        }
        if (requested < original) {  // if original size large enough
            void *to_return = memcpy(
                ret, to_save,
                requested);  // copies over memory, passing in requested size
            return to_return;
        }
        void *to_return =
            memcpy(ret, to_save,
                   original);  // copies over memory, passing in original size
        return to_return;
    }
}
