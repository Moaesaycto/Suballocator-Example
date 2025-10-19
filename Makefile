# COMP1521 22T1 --- Assignment 2: `Allocator', a simple sub-allocator 
# YOU DO NOT NEED TO MODIFY THIS FILE

CC	= dcc
CFLAGS	= -Wall -Werror -fno-strict-aliasing

.PHONY:	all
all:	test1 test2 test3

test1:		test1.o allocator.o
test1.o:	test1.c allocator.h

test2:		test2.o allocator.o
test2.o:	test2.c allocator.h

test3:		test3.o allocator.o
test3.o:	test3.c allocator.h

allocator.o:	allocator.c allocator.h


.PHONY: clean
clean:
	-rm -f test1 test1.o
	-rm -f test2 test2.o
	-rm -f test3 test3.o
	-rm -f allocator.o

.PHONY: give
give: allocator.c
	give cs1521 ass2_allocator $^
