#include <stdio.h>

#include "interfaces.h"

void iterm_readc(VM_Memory *ram);
void iterm_writec(VM_Memory *ram);
void iterm_getarg(int argn, VM_Memory *ram);

void iterm_update(VM_Memory *ram)
{
    int8_t fn_val = load_mem(PTR_TERMINAL_FN, 0, ram);
    switch (fn_val)
    {
        case NONE: break;
        case TERMINAL_READC: iterm_readc(ram); break;
        case TERMINAL_WRITEC: iterm_writec(ram); break;
        case TERMINAL_GETARG1: iterm_getarg(1, ram); break;
        case TERMINAL_GETARG2: iterm_getarg(2, ram); break;
        case TERMINAL_GETARG3: iterm_getarg(3, ram); break;
        case TERMINAL_GETARG4: iterm_getarg(4, ram); break;
    }
}

void iterm_readc(VM_Memory *ram)
{
    char c = getchar();
    store_mem(PTR_TERMINAL_READC, 0, (int8_t)c, ram);
    store_mem(PTR_TERMINAL_FN, 0, 0, ram);
}

void iterm_writec(VM_Memory *ram)
{
    char c = load_mem(PTR_TERMINAL_WRITEC, 0, ram);
    putchar(c);
    store_mem(PTR_TERMINAL_FN, 0, 0, ram);
}

void iterm_getarg(int argn, VM_Memory *ram)
{   
    char *arg = ram->argv[argn + 1];
    ptrid_t store_ptr = (ptrid_t)load_mem(PTR_TERMINAL_ARGS, 0, ram);

    int i = 0;
    while (*arg != '\0')
    {
        store_mem(store_ptr, i++, (int32_t)*arg, ram);
        arg++;
    }
    store_mem(PTR_TERMINAL_FN, 0, 0, ram);
}