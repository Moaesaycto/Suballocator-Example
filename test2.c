////////////////////////////////////////////////////////////////////////////////
// COMP1521 22T1 --- Assignment 2: `Allocator', a simple sub-allocator        //
// <https://www.cse.unsw.edu.au/~cs1521/22T1/assignments/ass2/index.html>     //
//                                                                            //
// 2021-04-06   v1.0    Team COMP1521 <cs1521 at cse.unsw.edu.au>             //
//                                                                            //
///////////////////// YOU DO NOT NEED TO MODIFY THIS FILE. /////////////////////

// test2.c: insert random values into a sorted linked list

#include <stdio.h>
#include <stdlib.h>

#include "allocator.h"

typedef struct _node {
    int data;
    struct _node *next;
} Node;

typedef Node *List;

List insert(List, int);
void show_list(List);
void free_list(List);

// This `#define' allows you to use your own `my_malloc' and `my_free'
// implementations -- comment it out or remove it to use plain old
// `malloc(3)' and `free(3)' and to see the expected behaviour.
#define USE_MY_ALLOCATOR

#ifdef USE_MY_ALLOCATOR
#define MEM_ALLOC my_malloc
#define MEM_FREE my_free
#else
#define MEM_ALLOC malloc
#define MEM_FREE free
#endif

int main(void) {
#ifdef USE_MY_ALLOCATOR
    init_heap(10000);
    dump_heap(2);
#endif

    List list = NULL;
    for (int i = 0; i < 100; i++) {
        list = insert(list, rand() % 100);
        printf("L = ");
        show_list(list);
#ifdef USE_MY_ALLOCATOR
        dump_heap(0);
#endif
    }

    free_list(list);
    printf("After free_list ...\n");

#ifdef USE_MY_ALLOCATOR
    dump_heap(1);
    free_heap();
#endif

    return 0;
}

List insert(List L, int n) {
    Node *new = MEM_ALLOC(sizeof(Node));
    if (new == NULL) {
        fprintf(stderr, "couldn't allocate Node\n");
        exit(1);
    }

    new->data = n;
    Node *prev = NULL;
    Node *curr = L;
    while (curr != NULL) {
        if (n < curr->data) {
            break;
        }

        prev = curr;
        curr = curr->next;
    }

    if (prev == NULL) { // new front
        new->next = curr;
        return new;
    } else { // insert in middle
        new->next = prev->next;
        prev->next = new;
        return L;
    }
}

void show_list(List L) {
    while (L != NULL) {
        printf("%d", L->data);

        if (L->next != NULL) {
            printf("->");
        }

        L = L->next;
    }

    printf("\n");
}

void free_list(List L) {
    if (L == NULL) {
        return;
    }

    free_list(L->next);
    MEM_FREE(L);
}
