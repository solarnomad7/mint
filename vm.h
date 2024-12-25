#pragma once

#include <stdint.h>

#define STACK_SIZE 0x100

typedef uint32_t address_t;

typedef enum Type { ARR8 = 8, ARR16 = 16, ARR32 = 32 } Type;

typedef struct Pointer
{
    address_t address;
    uint32_t size;
    Type type;
} Pointer;

typedef struct VM_Memory
{
    void **mem;
    Pointer **pointers;
    uint16_t num_pointers;
    uint32_t mem_size;
} VM_Memory;

typedef struct VM_Core
{
    int32_t stack[STACK_SIZE];
    int8_t sp;

    address_t ret_stack[STACK_SIZE];
    int8_t rp;

    int32_t loop_ivals[4];
    int32_t loop_maxvals[4];

    VM_Memory heap;
} VM_Core;

/**
 * Initializes the virtual machine and allocates memory on the heap.
 * @param data Valid Mint executable data
 * @param vm VM instance
 * @return Initial address
 */
uint16_t init_vm(int8_t data[], VM_Core *vm);

/**
 * Begins evaluating bytecode contained in memory from a given address.
 * @param address Initial address
 * @param vm VM instance
 */
void start_eval(address_t address, VM_Core *vm);

/**
 * Frees all heap memory.
 * @param vm VM instance
 */
void free_mem(VM_Core *vm);

/**
 * Displays all allocated memory in a table. Useful for debugging.
 * @param vm VM instance
 */
void display_mem(VM_Core *vm);