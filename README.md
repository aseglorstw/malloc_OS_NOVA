# NOVA Memory Management

This repository presents my implementations of the `malloc()` function and the `brk()` system call for the NOVA operating system.

## Memory Block Representation

To manage information about free memory blocks, I allocate one machine word for each block. This word includes the size of the block, rounded to the nearest machine word. The last bit in the header indicates whether the block is occupied (1) or free (0).

## Dynamic Memory Allocation

In cases of insufficient dynamic memory, the `brk()` system call is invoked to expand the size of the heap, ensuring proper memory allocation.

## Freeing Memory and Fragmentation Prevention

The repository also includes an implementation of the `free()` function. Apart from releasing memory blocks and performing error checks, this function intelligently combines adjacent free blocks to mitigate fragmentation issues.

This memory management system aims to provide efficient and effective memory allocation and deallocation within the NOVA operating system.

<p align="center">
  <img src="https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEgYzSvDO_onzcCa77y5P9u4Gq67gc2dIpSUbv2ljAfBj1w4icSJnOw9QGuO6s1H4TEfJRNs-_mzgVh_pTO5hEuI9xALBSg0L0QyDOAJlyA9FPCzSCeRrMXIGD-ejTn5uQDa2OMBLSeHyeI/s1600/Capture.JPG">
</p>
