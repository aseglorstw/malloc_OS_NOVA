#include "mem_alloc.h"
#include "types.h"
#include "stdio.h"

#define ERROR_MALLOC 0
#define ERROR_FREE -1

#define UNDEFINED 0

#define HEADER_SIZE sizeof(mword)
#define MARK_AS_ALLOCATED(SIZE) ((SIZE) | 1)
#define MARK_AS_DEALLOCATED(SIZE) ((SIZE) & ~1)
#define GET_SIZE(HEADER) ((HEADER) & ~1)
#define IS_ALLOCATED(HEADER) ((HEADER) & 1)
#define ROUND_UP_TO_MULTIPLE(SIZE) (((SIZE) + HEADER_SIZE - 1) / HEADER_SIZE * HEADER_SIZE)

typedef unsigned char *byte_pointer;

static inline unsigned syscall2(unsigned w0, unsigned w1)
{
    asm volatile("   mov %%esp, %%ecx    ;"
                 "   mov $1f, %%edx      ;"
                 "   sysenter            ;"
                 "1:                     ;"
            : "+a"(w0)
            : "S"(w1)
            : "ecx", "edx", "memory");
    return w0;
}

static void *nbrk(void *address) { return (void *)syscall2(3, (unsigned)address);}

byte_pointer break_min = NULL;

void *my_malloc(mword size) {

    if(break_min == NULL) { break_min = (byte_pointer)nbrk(0);}
    byte_pointer current_byte = break_min;
    byte_pointer current_break = nbrk(0);
    size = ROUND_UP_TO_MULTIPLE(size);

    while (current_byte < current_break) {
        mword header_of_block = *(mword*)current_byte;

        if (header_of_block == UNDEFINED || (!IS_ALLOCATED(header_of_block) && GET_SIZE(header_of_block) == size)) { break;}

        else if (!IS_ALLOCATED(header_of_block) && GET_SIZE(header_of_block) > size + HEADER_SIZE) {
            mword new_size_of_empty_block = GET_SIZE(header_of_block) - size - HEADER_SIZE;
            *((mword*)(current_byte + HEADER_SIZE + size)) = MARK_AS_DEALLOCATED(new_size_of_empty_block);
            break;
        }
        current_byte += GET_SIZE(header_of_block) + HEADER_SIZE;
    }

    if (current_byte + size + HEADER_SIZE >= current_break) {
        current_break = nbrk(current_byte + size  + HEADER_SIZE + 1);
        if (current_break == 0) { return ERROR_MALLOC;}
    }

    *(mword*)current_byte = MARK_AS_ALLOCATED(size);

    return current_byte + HEADER_SIZE;
}

int my_free(void *address) {
    if ((byte_pointer)address < break_min || address > nbrk(0) || !IS_ALLOCATED(*((mword *)(address - HEADER_SIZE)))) {
        return ERROR_FREE;
    }

    *((mword *)(address - HEADER_SIZE)) = MARK_AS_DEALLOCATED(*((mword *)(address - HEADER_SIZE)));

    byte_pointer current_byte = break_min;
    byte_pointer current_break = nbrk(0);
    while(current_byte < current_break) {
        mword header_of_block = *(mword*)current_byte;
        mword header_of_next_block = *((mword*)(current_byte + GET_SIZE(header_of_block) + HEADER_SIZE));
        if(header_of_block != UNDEFINED && !IS_ALLOCATED(header_of_block) && header_of_next_block != UNDEFINED && !IS_ALLOCATED(header_of_next_block)) {
            mword new_size_of_empty_block = GET_SIZE(header_of_block) + GET_SIZE(header_of_next_block) + HEADER_SIZE;
            *(mword*)current_byte = MARK_AS_DEALLOCATED(new_size_of_empty_block);
        }
        else { current_byte += GET_SIZE(header_of_block) + HEADER_SIZE;}
    }
    return 0;
}