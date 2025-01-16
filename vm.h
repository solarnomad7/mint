#pragma once

#include <stdint.h>

#define STACK_SIZE  0x100
#define MEM_SIZE    0x10000
#define PTRS_SIZE   0x200
#define PAGE_SIZE   0x40000

typedef uint32_t address_t;
typedef int16_t ptrid_t;

typedef enum Type { INT8 = 1, INT16 = 2, INT32 = 4 } Type;

typedef enum Error
{
    SEGMENTATION_FAULT  = 1,
    MAX_RECURSION_DEPTH = 2,
    STACK_OVERFLOW      = 3,
    STACK_UNDERFLOW     = 4,
} Error;

typedef struct Pointer
{
    address_t address;
    uint16_t size;
    Type type;
} Pointer;

typedef struct VM_Memory
{
    int8_t* mem;
    int8_t* page;
    Pointer* pointers;

    char** argv;
} VM_Memory;

typedef struct VM_Core
{
    int32_t stack[STACK_SIZE];
    address_t ret_stack[STACK_SIZE];

    int16_t sp;
    int16_t rp;

    int32_t loop_ivals[4];
    int32_t loop_maxvals[4];
    address_t loop_addrs[4];
    int8_t loop_depth;

    VM_Memory ram;
} VM_Core;

/**
 * @brief Checks file validity and initializes VM memory.
 *
 * @param data Raw executable data
 * @param vm Pointer to the virtual machine
 * @param argv Arguments
 * @return ptrid_t Mint pointer to the main word
 */
ptrid_t init_vm(int8_t data[], VM_Core *vm, char **argv);

/**
 * @brief Evaluates bytecode beginning from the given address until it reaches EOF
 * 
 * @param address Starting address
 * @param vm Pointer to the virtual machine
 * @return int Status code
 */
int eval(address_t address, VM_Core *vm);

/**
 * @brief Frees all memory
 * 
 * @param vm Pointer to the virtual machine
 */
void free_mem(VM_Core *vm);

/**
 * @brief Loads a value from memory.
 * 
 * @param ptr Pointer to a region in memory
 * @param idx Index of the value
 * @param ram Pointer to VM_Memory
 * @return int32_t Retrieved value
 */
int32_t load_mem(ptrid_t ptr, uint16_t idx, VM_Memory *ram);

/**
 * @brief Stores a value in memory.
 * 
 * @param ptr Pointer to a region in memory
 * @param idx Index to store the value at
 * @param val Value
 * @param ram Pointer to VM_Memory
 */
void store_mem(ptrid_t ptr, uint16_t idx, int32_t val, VM_Memory *ram);