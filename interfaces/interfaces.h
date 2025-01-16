#pragma once

#include "terminal.h"
#include "page.h"

#define NUM_INTERFACE_PTRS 8

enum InterfacePointers
{
    PTR_NULL                = 0,
    PTR_PAGE_FN             = 1,
    PTR_PAGE_RWPTR          = 2,
    PTR_PAGE_RWSIZE         = 3,
    PTR_TERMINAL_FN         = 4,
    PTR_TERMINAL_ARGS       = 5,
    PTR_TERMINAL_READC      = 6,
    PTR_TERMINAL_WRITEC     = 7,
};

enum InterfaceFunctions
{
    NONE                    = 0,
    PAGE_READ               = 1,
    PAGE_WRITE              = 2,
    TERMINAL_READC          = 1,
    TERMINAL_WRITEC         = 2,
    TERMINAL_GETARG1        = 3,
    TERMINAL_GETARG2        = 4,
    TERMINAL_GETARG3        = 5,
    TERMINAL_GETARG4        = 6,
};