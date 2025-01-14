#pragma once

#include "terminal.h"

#define NUM_INTERFACE_PTRS 4

enum InterfacePointers
{
    PTR_TERMINAL_FN         = 0,
    PTR_TERMINAL_ARGS       = 1,
    PTR_TERMINAL_READC      = 2,
    PTR_TERMINAL_WRITEC     = 3,
};

enum InterfaceFunctions
{
    NONE                    = 0,
    TERMINAL_READC          = 1,
    TERMINAL_WRITEC         = 2,
    TERMINAL_GETARG1        = 3,
    TERMINAL_GETARG2        = 4,
    TERMINAL_GETARG3        = 5,
    TERMINAL_GETARG4        = 6,
};