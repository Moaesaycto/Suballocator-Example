# **Suballocator Example (C)**

**Type:** Memory Management System · **Tech Stack:** C · **Status:** Completed

## **Overview**

A simple **suballocator** implemented in C for the **COMP1521** course, designed to manage dynamic memory manually without relying on the system allocator. It mimics how an OS or runtime allocator tracks and manages free and allocated blocks in a heap.

## **Key Features**

* Implements a fixed-size heap (`init_heap`) with headers marking **allocated** and **free** chunks.
* Supports:

  * `my_malloc()`: allocates memory, splitting free chunks when needed.
  * `my_free()`: deallocates memory and merges adjacent free blocks to prevent fragmentation.
  * `heap_offset()`: computes an object’s offset from heap start.
  * `dump_heap()`: displays heap state for debugging.
* Uses a **free list** for fast lookup and management of available chunks.
* Handles **coalescing**, **alignment**, and **boundary validation** efficiently.
* Enforces strict validation with detailed error handling on invalid frees.

## **Purpose**

Developed to understand **low-level memory management**, **pointer arithmetic**, and **heap structure manipulation**; effectively reimplementing core concepts behind `malloc` and `free`.
