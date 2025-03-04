#pragma once

#include "terminal.h"
#include "fs.h"

#define NUM_INTERFACE_PTRS 9

enum InterfacePointers
{
    PTR_NULL                = 0,
    PTR_TERMINAL_FN         = 1,
    PTR_TERMINAL_ARGS       = 2,
    PTR_TERMINAL_READC      = 3,
    PTR_TERMINAL_WRITEC     = 4,
    PTR_FS_FN               = 5,
    PTR_FS_PATH             = 6,
    PTR_FS_SOURCE           = 7,
    PTR_FS_DEST             = 8,
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
    FS_READ                 = 1,
    FS_WRITE                = 2,
    FS_APPEND               = 3,
    FS_FILELEN              = 4,
};