This repository contains my implementations of the `malloc()` function and the `brk()` system call for the NOVA operating system.

To store information about free memory blocks, I allocate one machine word for each block, containing the size of the block, rounded to the nearest machine word. The last bit in the header is used to indicate whether the block is occupied (1) or free (0).

If there is insufficient dynamic memory, the `brk()` system call is invoked to increase the size of the heap.

Additionally, there is an implementation of the `free()` function, which, apart from releasing memory blocks and error checking, combines adjacent free blocks to prevent fragmentation.
