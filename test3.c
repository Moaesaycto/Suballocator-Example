////////////////////////////////////////////////////////////////////////////////
// COMP1521 22T1 --- Assignment 2: `Allocator', a simple sub-allocator        //
// <https://www.cse.unsw.edu.au/~cs1521/22T1/assignments/ass2/index.html>     //
//                                                                            //
// 2021-04-06   v1.0    Team COMP1521 <cs1521 at cse.unsw.edu.au>             //
//                                                                            //
///////////////////// YOU DO NOT NEED TO MODIFY THIS FILE. /////////////////////

// test3.c: read allocate/free ops and do them

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocator.h"

static void run_command(void **vars, const char *line);
static void run_allocate(void **vars, char var, int size);
static void run_free(void **vars, char var);
static int valid_var(char c);
static void dump_vars(void **vars);

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

    // print initial state of heap
    dump_heap(2);

    // array of 26 pointer variables
    void *vars[26];
    for (int i = 0; i < 26; i++) {
        vars[i] = NULL;
    }

    // read malloc/free commands and carry them out
    char line[BUFSIZ];
    while (fgets(line, BUFSIZ, stdin) != NULL) {
        // terminate string at newline
        line[strcspn(line, "\n")] = '\0';

        run_command(vars, line);

        dump_vars(vars);
        dump_heap(1);
    }

    // print final state of heap
    printf("\non exit:\n");
    dump_heap(2);

    return 0;
}

static void run_command(void **vars, const char *line) {
    char var;
    int size;

    if (sscanf(line, "%c = allocate %d", &var, &size) == 2) {
        if (!valid_var(var)) {
            fprintf(stderr, "invalid variable `%c'\n", var);
        } else {
            run_allocate(vars, var, size);
        }
    } else if (sscanf(line, "free %c", &var) == 1) {
        if (!valid_var(var)) {
            fprintf(stderr, "invalid variable `%c'\n", var);
        } else {
            run_free(vars, var);
        }
    } else {
        fprintf(stderr, "ignoring unknown command: %s\n", line);
    }
}

static void run_allocate(void **vars, char var, int size) {
    printf("\n%c = allocate(%d);\n", var, size);

    if ((vars[var - 'a'] = my_malloc(size)) == NULL) {
        printf("couldn't allocate %d bytes for `%c'\n", size, var);
        exit(1);
    }
}

static void run_free(void **vars, char var) {
    printf("\nfree(%c);\n", var);

    my_free(vars[var - 'a']);
//    don't
//    vars[var - 'a'] = NULL;
}

static int valid_var(char c) {
    return 'a' <= c && c <= 'z';
}

// prints allocated variables
// may be helpful for debugging
static void dump_vars(void **vars) {
    for (int i = 0; i < 26; i++) {
        if (vars[i] != NULL) {
            printf("variable %c is at heap offset %5ld\n", 'a' + i, heap_offset(vars[i]));
        }
    }
}
