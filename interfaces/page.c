#include "interfaces.h"

void ipage_read(VM_Memory *ram);
void ipage_write(VM_Memory *ram);

void ipage_update(VM_Memory *ram)
{
    int8_t fn_val = load_mem(PTR_PAGE_FN, 0, ram);
    switch (fn_val)
    {
        case NONE: break;
        case PAGE_READ: ipage_read(ram); break;
        case PAGE_WRITE: ipage_write(ram); break;
    }
}

void ipage_read(VM_Memory *ram)
{
    int32_t read_idx = load_mem(PTR_PAGE_RWSIZE, 0, ram);
    int32_t read_len = load_mem(PTR_PAGE_RWSIZE, 1, ram);
    address_t write_addr = ram->pointers[load_mem(PTR_PAGE_RWPTR, 0, ram)].address;

    for (int i = 0; i < read_len; i++)
    {
        ram->mem[write_addr++] = ram->page[read_idx + i];
    }
    store_mem(PTR_PAGE_FN, 0, 0, ram);
}

void ipage_write(VM_Memory *ram)
{
    int32_t write_idx = load_mem(PTR_PAGE_RWSIZE, 0, ram);
    int32_t write_len = load_mem(PTR_PAGE_RWSIZE, 1, ram);
    address_t read_addr = ram->pointers[load_mem(PTR_PAGE_RWPTR, 0, ram)].address;

    for (int i = 0; i < write_len; i++)
    {
        ram->page[write_idx + i] = ram->mem[read_addr++];
    }
    store_mem(PTR_PAGE_FN, 0, 0, ram);
}