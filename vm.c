#include <stdio.h>
#include <stdlib.h>

#include "vm.h"
#include "op_table.h"

#define PUSH(x)     if (vm->sp == STACK_SIZE-1) { return STACK_OVERFLOW; } vm->sp++; vm->stack[vm->sp] = x
#define POP(x)      if (vm->sp == -1) { return STACK_UNDERFLOW; } int32_t x = vm->stack[vm->sp--]
#define get_mem(a)  vm->ram.mem[a]

int32_t load_mem(ptrid_t ptr, uint16_t idx, VM_Memory *ram);
void store_mem(ptrid_t ptr, uint16_t idx, int32_t val, VM_Memory *ram);

static int16_t c_to_s(int8_t a, int8_t b);
static int32_t c_to_i(int8_t a, int8_t b, int8_t c, int8_t d);

ptrid_t init_vm(int8_t data[], VM_Core *vm)
{
    if (!(data[0] == 'M' && data[1] == 'I' && data[2] == 'N' && data[3] == 'T')) return -1; // File validity check

    vm->sp = -1;
    vm->rp = -1;
    vm->loop_depth = -1;

    if ((vm->ram.mem = calloc(MEM_SIZE, sizeof(int8_t))) == NULL) return -1;
    if ((vm->ram.pointers = malloc(PTRS_SIZE * sizeof(Pointer))) == NULL) return -1;

    ptrid_t initial_ptr = c_to_s(data[6], data[7]);

    int num_labels = c_to_s(data[8], data[9]);

    int i = 12;
    for (int label_i = 0; label_i < num_labels; label_i++) // Skip metadata
    {
        int8_t label_len = data[i];
        i += label_len + 1;
        if (label_i < num_labels - 1) i += 2;
    }

    ptrid_t curr_ptr = 0;
    address_t next_free_addr = 0;

    while (data[i] != END_FILE)
    {
        if (data[i] == DEF)
        {
            i++;
            int8_t type = data[i++];

            int8_t tmp_size = data[i++];
            uint16_t size = c_to_s(tmp_size, data[i++]);

            int8_t tmp_id = data[i++];
            curr_ptr = c_to_s(tmp_id, data[i++]);

            int8_t tmp_ptrs = data[i++];
            uint16_t num_pointers = c_to_s(tmp_ptrs, data[i++]);

            i += num_pointers * 2;

            vm->ram.pointers[curr_ptr].address = next_free_addr;
            vm->ram.pointers[curr_ptr].type = type;
            vm->ram.pointers[curr_ptr].size = size;

            next_free_addr += size;

            int j = 0;
            while (data[i] != END)
            {
                vm->ram.mem[vm->ram.pointers[curr_ptr].address + j++] = data[i++];
            }
            vm->ram.mem[vm->ram.pointers[curr_ptr].address + j] = END;
        }
        else i++;
    }

    return initial_ptr;
}

int eval(address_t address, VM_Core *vm)
{
    int halt = 0;
    while (!halt)
    {
        switch (vm->ram.mem[address])
        {
            case END_FILE:
            case HALT:      { halt = 1; break; }
            case END:       {
                if (vm->rp > -1)
                {
                    address = vm->ret_stack[vm->rp--];
                    continue;
                }
                else halt = 1;
                break;
            }
            case PUSH8:     { PUSH(get_mem(++address)); break; }
            case PUSH16:    {
                int8_t a = get_mem(++address);
                PUSH(c_to_s(a, get_mem(++address)));
                break;
            }
            case PUSH32:    {
                int8_t a = get_mem(++address);
                int8_t b = get_mem(++address);
                int8_t c = get_mem(++address);
                PUSH(c_to_i(a, b, c, get_mem(++address)));
                break;
            }
            case CALL:      {
                POP(p);
                if (vm->rp == STACK_SIZE-1) { return MAX_RECURSION_DEPTH; }
                vm->ret_stack[++(vm->rp)] = address + 1;
                address = vm->ram.pointers[p].address;
                continue;
            }
            case IF:        {
                POP(v);
                if (v) break;
                int cond_depth = 1;
                while (cond_depth > 0) // Skip over the if block
                {
                    int8_t op = get_mem(++address);
                    if (op == IF) cond_depth++;
                    else if (op == ENDIF) cond_depth--;
                    else if (op == ELSE && cond_depth == 1) break;
                }
                break;
            }
            case ELSE:      { while (get_mem(++address) != ENDIF) {}; break; }
            case LOOP:      { vm->loop_addrs[++(vm->loop_depth)] = address + 1; break; }
            case REPEAT:    { address = vm->loop_addrs[vm->loop_depth]; continue; }
            case FOR:       {
                vm->loop_addrs[++(vm->loop_depth)] = address + 1;
                POP(end); vm->loop_maxvals[vm->loop_depth] = end + 1;
                POP(init); vm->loop_ivals[vm->loop_depth] = init;
                break;
            }

            case NEXT:      {
                if (vm->loop_ivals[vm->loop_depth] == vm->loop_maxvals[vm->loop_depth]) { vm->loop_depth--; break; }
                else { address = vm->loop_addrs[vm->loop_depth]; continue; }
            }
            case ADDI:      { POP(i); vm->loop_ivals[vm->loop_depth] += i; break; }
            case PUSHI:     { POP(i); PUSH(vm->loop_ivals[i]); break; }
            case BREAK:     {
                int curr_loop_depth = vm->loop_depth;
                while (vm->loop_depth >= curr_loop_depth) // Ignore any possible nested loops
                {
                    int8_t op = get_mem(++address);
                    if (op == FOR || op == LOOP) vm->loop_depth++;
                    else if (op == NEXT || op == REPEAT) vm->loop_depth--;
                }
                break;
            }
            case LOAD:      {
                POP(i); POP(p);
                if (vm->ram.pointers[p].size <= i) { return SEGMENTATION_FAULT; }
                PUSH(load_mem(p, (uint16_t)i, &(vm->ram)));
                break;
            }
            case STORE:     {
                POP(v); POP(i); POP(p);
                if (vm->ram.pointers[p].size <= i) { return 1; }
                store_mem(p, (uint16_t)i, v, &(vm->ram));
                break;
            }
            case ADDR:      { POP(p); PUSH(vm->ram.pointers[p].address); break; }
            case SIZE:      { POP(p); PUSH(vm->ram.pointers[p].size); break; }
            case BYTES:     { POP(p); PUSH(vm->ram.pointers[p].type); break; }
            case POP:       { POP(a); break; }
            case DUP:       { PUSH(vm->stack[vm->sp-1]); break; }
            case SWAP:      { POP(a); POP(b); PUSH(a); PUSH(b); break; }
            case OVER:      { POP(a); POP(b); PUSH(b); PUSH(a); PUSH(b); break; }
            case ROT:       { POP(a); POP(b); POP(c); PUSH(b); PUSH(a); PUSH(c); break; }
            case OUTINT:    { POP(v); printf("%d", v); break; }
            case OUTCHAR:   { POP(c); printf("%c", c); break; }
            case ADD:       { POP(a); POP(b); PUSH(a+b); break; }
            case SUB:       { POP(a); POP(b); PUSH(b-a); break; }
            case MUL:       { POP(a); POP(b); PUSH(a*b); break; }
            case DIV:       { POP(a); POP(b); PUSH(b/a); break; }
            case MOD:       { POP(a); POP(b); PUSH(b%a); break; }
            case EQU:       { POP(a); POP(b); PUSH(a == b); break; }
            case GREATER:   { POP(a); POP(b); PUSH(b > a); break; }
            case LESS:      { POP(a); POP(b); PUSH(b < a); break; }
            case GEQ:       { POP(a); POP(b); PUSH(b >= a); break; }
            case LEQ:       { POP(a); POP(b); PUSH(b <= a); break; }
            case INVERT:    { POP(a); PUSH(~a); break; }
            case AND:       { POP(a); POP(b); PUSH(a & b); break; }
            case OR:        { POP(a); POP(b); PUSH(a | b); break; }
            case XOR:       { POP(a); POP(b); PUSH(a ^ b); break; }
            case SLEFT:     { POP(a); POP(b); PUSH(b << a); break; }
            case SRIGHT:    { POP(a); POP(b); PUSH(b >> a); break; }
        }
        address++;
    }
    return 0;
}

int32_t load_mem(ptrid_t ptr, uint16_t idx, VM_Memory *ram)
{
    int8_t type = ram->pointers[ptr].type;
    address_t addr = ram->pointers[ptr].address + type * idx;

    if (type == INT8)
    {
        return ram->mem[addr];
    }
    else if (type == INT16)
    {
        int8_t a = ram->mem[addr++];
        return c_to_s(a, ram->mem[addr]);
    }
    else if (type == INT32)
    {
        int8_t a = ram->mem[addr++];
        int8_t b = ram->mem[addr++];
        int8_t c = ram->mem[addr++];
        return c_to_i(a, b, c, ram->mem[addr]);
    }
    return 0;
}

void store_mem(ptrid_t ptr, uint16_t idx, int32_t val, VM_Memory *ram)
{
    uint32_t uval = (uint32_t)val;
    int8_t type = ram->pointers[ptr].type;
    address_t addr = ram->pointers[ptr].address + type * idx;

    if (type == INT8)
    {
        ram->mem[addr] = (int8_t)uval;
    }
    else if (type == INT16)
    {
        ram->mem[addr++] = (int8_t)((uval >> 8) & 0xFF);
        ram->mem[addr] = (int8_t)(uval & 0xFF);
    }
    else if (type == INT32)
    {
        ram->mem[addr++] = (int8_t)((uval >> 24) & 0xFF);
        ram->mem[addr++] = (int8_t)((uval >> 16) & 0xFF);
        ram->mem[addr++] = (int8_t)((uval >> 8) & 0xFF);
        ram->mem[addr] = (int8_t)(uval & 0xFF);
    }
}

void free_mem(VM_Core *vm)
{
    free(vm->ram.mem);
    free(vm->ram.pointers);
}

static int16_t c_to_s(int8_t a, int8_t b)
{
    return ((uint8_t)a << 8) | (uint8_t)b;
}

static int32_t c_to_i(int8_t a, int8_t b, int8_t c, int8_t d)
{
    return ((uint8_t)a) << 24 | ((uint8_t)b) << 16 | ((uint8_t)c) << 8 | (uint8_t)d;
}