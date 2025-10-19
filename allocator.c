////////////////////////////////////////////////////////////////////////////////
// COMP1521 22T1 --- Assignment 2: `Allocator', a simple sub-allocator        //
// <https://www.cse.unsw.edu.au/~cs1521/22T1/assignments/ass2/index.html>     //
//                                                                            //
// Written by STEPHEN-LERANTGES (z5319858) on 16/04/2022.                     //
//                                                                            //
// 2021-04-06   v1.0    Team COMP1521 <cs1521 at cse.unsw.edu.au>             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocator.h"

// DO NOT CHANGE CHANGE THESE #defines

/** minimum total space for heap */
#define MIN_HEAP 4096

/** minimum amount of space to split for a free chunk (excludes header) */
#define MIN_CHUNK_SPLIT 32

/** the size of a chunk header (in bytes) */
#define HEADER_SIZE (sizeof(struct header))

/** constants for chunk header's status */
#define ALLOC 0x55555555
#define FREE 0xAAAAAAAA

// ADD ANY extra #defines HERE

// DO NOT CHANGE these struct defintions

typedef unsigned char byte;

/** The header for a chunk. */
typedef struct header {
    uint32_t status; /**< the chunk's status -- shoule be either ALLOC or FREE */
    uint32_t size;   /**< number of bytes, including header */
    byte     data[]; /**< the chunk's data -- not interesting to us */
} header_type;


/** The heap's state */
typedef struct heap_information {
    byte      *heap_mem;      /**< space allocated for Heap */
    uint32_t   heap_size;     /**< number of bytes in heap_mem */
    byte     **free_list;     /**< array of pointers to free chunks */
    uint32_t   free_capacity; /**< maximum number of free chunks (maximum elements in free_list[]) */
    uint32_t   n_free;        /**< current number of free chunks */
} heap_information_type;

// Footnote:
// The type unsigned char is the safest type to use in C for a raw array of bytes
//
// The use of uint32_t above limits maximum heap size to 2 ** 32 - 1 == 4294967295 bytes
// Using the type size_t from <stdlib.h> instead of uint32_t allowing any practical heap size,
// but would make struct header larger.


// DO NOT CHANGE this global variable
// DO NOT ADD any other global  variables

/** Global variable holding the state of the heap */
static struct heap_information my_heap;

// ADD YOUR FUNCTION PROTOTYPES HERE
static int search_next_index(uint32_t size);
static void free_error();
static header_type *next_addr(uintptr_t current_addr, uintptr_t max_addr, header_type *current_chunk);
static void free_merge(header_type *prev, header_type *curr, header_type *next, header_type *prev_free);
static int index_of(header_type *chunk);
static void pop_free_list(int index);
static void push_n(int n, header_type *curr);


// Initialise my_heap
int init_heap(uint32_t size) {
    if (size < MIN_HEAP) {
        size = MIN_HEAP;
    }

    if (size % 4 != 0) {
        size += (4 - (size % 4));
    }

    my_heap.heap_mem = malloc(size);
    if (my_heap.heap_mem == NULL) {
        return -1;
    }

    header_type *iheader = (header_type *) my_heap.heap_mem;
    iheader->size = size;
    iheader->status = FREE;


    // Where the data/header information is stored
    my_heap.free_list = malloc(size / HEADER_SIZE * sizeof(void *));
    if (my_heap.free_list == NULL) {
        return -1;
    }

    my_heap.free_list[0] = (byte *) iheader;

    my_heap.heap_size = size;
    my_heap.n_free = 1;
    my_heap.free_capacity = size / HEADER_SIZE;
    return 0;
}


// Allocate a chunk of memory large enough to store `size' bytes
void *my_malloc(uint32_t size) {
    if (size < 1) {
        return NULL;
    }

    if (size % 4 != 0) {
        size += (4 - (size % 4));
    }

    // Locate the next free chunk of size = size + HEADER_SIZE
    int index = search_next_index(size + HEADER_SIZE);
    if (index < 0) {
        return NULL;
    }
    header_type *selected_chunk = (header_type *) my_heap.free_list[index];

    // Allocating the whole chunk
    if (selected_chunk->size < size + HEADER_SIZE + MIN_CHUNK_SPLIT) {
        selected_chunk->status = ALLOC;

        // Remove the free chunk from the heap
        pop_free_list(index);
        my_heap.n_free--;
    }

    // Splitting then allocating
    else {
        int old_chunk_size = selected_chunk->size;
        header_type *new_free = (header_type *) (((uintptr_t) selected_chunk) + size + HEADER_SIZE);
        new_free->status = FREE;
        new_free->size = old_chunk_size - size - HEADER_SIZE;

        selected_chunk->size = size + HEADER_SIZE;
        selected_chunk->status = ALLOC;

        my_heap.free_list[index] = (byte *) new_free;
    }

    return (void *) ((uintptr_t) selected_chunk) + HEADER_SIZE;
}


// Deallocate chunk of memory referred to by `ptr'
void my_free(void *ptr) {
    if (ptr == NULL) {
        free_error();
        return;
    }

    // Finding the addresses of the previous and next chunk to prt
    uintptr_t max_addr = (uintptr_t) my_heap.heap_mem + my_heap.heap_size;
    uintptr_t curr_addr = (uintptr_t) my_heap.heap_mem;

    header_type *prev_chunk = NULL;
    header_type *next_chunk = NULL;
    header_type *prev_free = NULL;
    while (curr_addr < max_addr) {
        header_type *selected_chunk = (header_type *) curr_addr;

        // ptr points to the DATA, so verify if the selected_chunk's data
        if (&(selected_chunk->data) == ptr) {
            next_chunk = next_addr(curr_addr, max_addr, selected_chunk); // returns NULL if no next
            free_merge(prev_chunk, selected_chunk, next_chunk, prev_free);
            return;
        }

        if (selected_chunk->status == FREE) {
            prev_free = selected_chunk;
        }
        prev_chunk = selected_chunk;    // set previous to selected for next iter
        curr_addr += selected_chunk->size;   // moving the address up by selected's size

    }
    free_error();

}


static void free_merge(header_type *prev, header_type *curr, header_type *next, header_type *prev_free) {
    if (curr == NULL || curr->status == FREE) {
        free_error();
    }

    // if prev and next are free
    if ((prev != NULL && prev->status == FREE) && (next != NULL && next->status == FREE)) {
        pop_free_list(index_of(next));
        prev->size += curr->size + next->size;
        my_heap.n_free--; // we go from 2 free to 1.
    }

    // if prev is free
    else if ((prev != NULL && prev->status == FREE)) {
        prev->size += curr->size;
        prev->status = FREE;
    }

    // if next is free
    else if ((next != NULL && next->status == FREE)) {
        curr->status = FREE;
        curr->size += next->size;
        int j = index_of(next);
        my_heap.free_list[j] = (byte *) curr;
    }

    else {
        curr->status = FREE;
        my_heap.n_free++;
        int index;
        if (prev_free == NULL) {
            index = 0;
            push_n(index, curr);
        }
        else {
            index = index_of(prev_free);
            push_n(index + 1, curr);
        }
    }

}


// DO NOT CHANGE CHANGE THiS FUNCTION
//
// Release resources associated with the heap
void free_heap(void) {
    free(my_heap.heap_mem);
    free(my_heap.free_list);
}


// DO NOT CHANGE CHANGE THiS FUNCTION

// Given a pointer `obj'
// return its offset from the heap start, if it is within heap
// return -1, otherwise
// note: int64_t used as return type because we want to return a uint32_t bit value or -1
int64_t heap_offset(void *obj) {
    if (obj == NULL) {
        return -1;
    }
    int64_t offset = (byte *)obj - my_heap.heap_mem;
    if (offset < 0 || offset >= my_heap.heap_size) {
        return -1;
    }

    return offset;
}


// DO NOT CHANGE CHANGE THiS FUNCTION
//
// Print the contents of the heap for testing/debugging purposes.
// If verbosity is 1 information is printed in a longer more readable form
// If verbosity is 2 some extra information is printed
void dump_heap(int verbosity) {

    if (my_heap.heap_size < MIN_HEAP || my_heap.heap_size % 4 != 0) {
        printf("ndump_heap exiting because my_heap.heap_size is invalid: %u\n", my_heap.heap_size);
        exit(1);
    }

    if (verbosity > 1) {
        printf("heap size = %u bytes\n", my_heap.heap_size);
        printf("maximum free chunks = %u\n", my_heap.free_capacity);
        printf("currently free chunks = %u\n", my_heap.n_free);
    }

    // We iterate over the heap, chunk by chunk; we assume that the
    // first chunk is at the first location in the heap, and move along
    // by the size the chunk claims to be.

    uint32_t offset = 0;
    int n_chunk = 0;
    while (offset < my_heap.heap_size) {
        struct header *chunk = (struct header *)(my_heap.heap_mem + offset);

        char status_char = '?';
        char *status_string = "?";
        switch (chunk->status) {
        case FREE:
            status_char = 'F';
            status_string = "free";
            break;

        case ALLOC:
            status_char = 'A';
            status_string = "allocated";
            break;
        }

        if (verbosity) {
            printf("chunk %d: status = %s, size = %u bytes, offset from heap start = %u bytes",
                    n_chunk, status_string, chunk->size, offset);
        } else {
            printf("+%05u (%c,%5u) ", offset, status_char, chunk->size);
        }

        if (status_char == '?') {
            printf("\ndump_heap exiting because found bad chunk status 0x%08x\n",
                    chunk->status);
            exit(1);
        }

        offset += chunk->size;
        n_chunk++;

        // print newline after every five items
        if (verbosity || n_chunk % 5 == 0) {
            printf("\n");
        }
    }

    // add last newline if needed
    if (!verbosity && n_chunk % 5 != 0) {
        printf("\n");
    }

    if (offset != my_heap.heap_size) {
        printf("\ndump_heap exiting because end of last chunk does not match end of heap\n");
        exit(1);
    }

}

// ADD YOUR EXTRA FUNCTIONS HERE

// adds a header to my_heap.free_list's ith element.
static void push_n(int n, header_type *curr) {
    if (n < 0) {
        return;
    }
    for (int i = my_heap.n_free; i > n; i--) { 
        my_heap.free_list[i] = my_heap.free_list[i-1];
    }
    my_heap.free_list[n] = (byte *) curr;
}


// removes the ith position of my_heap.free_list
static void pop_free_list(int index) {
    for (int i = index; i < my_heap.n_free; i++) { 
        my_heap.free_list[i] = my_heap.free_list[i + 1];
    }
}


// Locates the index for the next available free chunk
// in the free_list element of the heap
static int search_next_index(uint32_t size) {
    header_type *curr = NULL;
    for (int index = 0; index < my_heap.n_free; index++) {
        curr = (header_type *) my_heap.free_list[index];
        if (curr->size >= size) {
            return index;
        }
    }
    return -1;
}


// finds the index of a chunk in free_list
// returns -1 if nothing was found
static int index_of(header_type *chunk) {
    for (int index = 0; index < my_heap.n_free; index++) {
        if (my_heap.free_list[index] == (byte *) chunk) {
            return index;
        }
    }
    return -1;
}


// Runs error for failed my_free (designated in spec)
static void free_error() {
    fprintf(stderr, "Attempt to free unallocated chunk\n");
    exit(1);
}


// Locates the next chunk in the heap. Returns NULL if there
// is no next.
static header_type *next_addr(uintptr_t current_addr, uintptr_t max_addr, header_type *current_chunk) {
    if (current_addr + current_chunk->size < max_addr) {
        return (header_type *) (current_addr + current_chunk->size);
    }
    return NULL;
}
