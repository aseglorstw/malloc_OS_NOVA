#include "ec.h"
#include "bits.h"
#include "ptab.h"
#include "stdio.h"
#include "string.h"

typedef enum {
    sys_print      = 1,
    sys_sum        = 2,
    sys_break      = 3,
    sys_thr_create = 4,
    sys_thr_yield  = 5,
} Syscall_numbers;

#define BREAK_MAX 0xBFFFF000
#define ERROR 0

void Ec::syscall_handler (uint8 a){
    Sys_regs * r = current->sys_regs();
    Syscall_numbers number = static_cast<Syscall_numbers> (a);
    switch (number) {
        case sys_print: {
            char *data = reinterpret_cast<char*>(r->esi);
            unsigned len = r->edi;
            for (unsigned i = 0; i < len; i++)
                printf("%c", data[i]);
            break;
        }
        case sys_sum: {
            int first_number = r->esi;
            int second_number = r->edi;
            r->eax = first_number + second_number;
            break;
        }
        case sys_break: {
            mword break_current = current->break_current;
            mword align_break_current = align_up(break_current, PAGE_SIZE);

            mword break_min = current->break_min;

            mword address = r->esi;
            mword align_address = align_up(address, PAGE_SIZE);

            mword attr = Ptab::PRESENT | Ptab::USER | Ptab::RW;
            r->eax = break_current;

            if(address == 0) { ret_user_sysexit();}

            else if(address < break_min || address > BREAK_MAX) {
                r->eax = ERROR;
                ret_user_sysexit();
            }

            if (address > break_current && align_break_current == align_address) {
                memset(reinterpret_cast<void*>(break_current), 0, static_cast<size_t>(address - break_current));
            }

            else if (address > break_current && align_break_current < align_address) {
                memset(reinterpret_cast<void*>(break_current), 0, static_cast<size_t>(align_break_current - break_current));
            }

            while (align_break_current < align_address) {
                void *page = Kalloc::allocator.alloc_page(1, Kalloc::FILL_0);
                if (!page) {
                    printf("ERROR in alloc_page: Not enough memory\n");
                    r->eax = ERROR;
                    break;
                }
                mword phys_page = Kalloc::virt2phys(page);
                if(!Ptab::insert_mapping(align_break_current, phys_page, attr)){
                    printf("ERROR in 'insert_mapping': Not enough memory\n");
                    r->eax = ERROR;
                    break;
                }
                align_break_current += PAGE_SIZE;
            }

            if(r->eax == ERROR) {
                mword original_align_break_current = align_up(break_current, PAGE_SIZE);
                while(align_break_current > original_align_break_current) {
                    align_break_current -= PAGE_SIZE;
                    mword phys_page = Ptab::get_mapping(align_break_current) & ~PAGE_MASK;
                    void *virt_page = Kalloc::phys2virt(phys_page);
                    Kalloc::allocator.free_page(virt_page);
                    mword* pdir = static_cast<mword*>(Kalloc::phys2virt(Cpu::cr3()));
                    mword* ptab = static_cast<mword*>(Kalloc::phys2virt(pdir[align_break_current >> 22] & ~PAGE_MASK));
                    ptab[(align_break_current >> PAGE_BITS) & 0x3ff] = 0;
                    Cpu::flush();
                }
                ret_user_sysexit();
            }

            while(align_break_current > align_address) {
                align_break_current -= PAGE_SIZE;
                mword phys_page = Ptab::get_mapping(align_break_current) & ~PAGE_MASK;
                void *virt_page = Kalloc::phys2virt(phys_page);
                Kalloc::allocator.free_page(virt_page);
                mword* pdir = static_cast<mword*>(Kalloc::phys2virt(Cpu::cr3()));
                mword* ptab = static_cast<mword*>(Kalloc::phys2virt(pdir[align_break_current >> 22] & ~PAGE_MASK));
                ptab[(align_break_current >> PAGE_BITS) & 0x3ff] = 0;
                Cpu::flush();
            }

            Ec::break_current = address;
            break;
        }
        default:
            printf ("unknown syscall %d\n", number);
            break;
    };
    ret_user_sysexit();
}
