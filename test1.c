////////////////////////////////////////////////////////////////////////////////
// COMP1521 22T1 --- Assignment 2: `Allocator', a simple sub-allocator        //
// <https://www.cse.unsw.edu.au/~cs1521/22T1/assignments/ass2/index.html>     //
//                                                                            //
// 2021-04-06   v1.0    Team COMP1521 <cs1521 at cse.unsw.edu.au>             //
//                                                                            //
///////////////////// YOU DO NOT NEED TO MODIFY THIS FILE. /////////////////////

// test1.c: init and dump heap.

#include <stdio.h>
#include <stdlib.h>
#include "allocator.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <size>\n", argv[0]);
        exit(1);
    }

    int heap_size = atoi(argv[1]);
    if (init_heap(heap_size) < 0) {
        fprintf(stderr, "can't init heap of size %d\n", heap_size);
        exit(1);
    }

    dump_heap(1);

    return 0;
}
