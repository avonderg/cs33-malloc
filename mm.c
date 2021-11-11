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
int mm_init(void) { return -1; }

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
    // TODO
    return NULL;
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
    // TODO
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
