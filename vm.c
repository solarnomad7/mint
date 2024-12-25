#include <stdio.h>
#include <stdlib.h>

#include "vm.h"
#include "op_table.h"

#define PUSH(x)             vm->sp++; vm->stack[vm->sp] = x
#define POP(x)              int32_t x = vm->stack[vm->sp--]
#define get_mem(addr)       *((int8_t*)(vm->heap.mem[addr]))

static void alloc_array(uint8_t bits, uint16_t id, uint16_t size, VM_Memory *heap, int *next_free);
static int init_array_data(uint8_t bits, uint16_t id, int8_t data[], int i, VM_Memory *heap);
static void define_word(uint16_t id, int8_t *data, VM_Memory *heap, int *next_free);

static int16_t c_to_s(int8_t a, int8_t b);
static int32_t c_to_i(int8_t a, int8_t b, int8_t c, int8_t d);

uint16_t init_vm(int8_t data[], VM_Core *vm)
{
    if (!(data[0] == 'M' && data[1] == 'I' && data[2] == 'N' && data[3] == 'T')) return -1; // File validity check

    vm->sp = -1;
    vm->rp = -1;

    vm->heap.num_pointers = c_to_s(data[4], data[5]);
    vm->heap.pointers = malloc(sizeof(Pointer*) * vm->heap.num_pointers);
    for (int i = 0; i < vm->heap.num_pointers; i++)
    {
        vm->heap.pointers[i] = malloc(sizeof(Pointer));
    }

    uint32_t mem_size = c_to_i(data[6], data[7], data[8], data[9]);
    vm->heap.mem_size = mem_size;
    vm->heap.mem = calloc(mem_size, sizeof(void*)); // Allocate the specified number of void pointers

    uint16_t initial_ptr = c_to_s(data[10], data[11]); // Pointer to the main word

    int next_free = 0;
    int i = 12;
    int inDef = 0;
    while (data[i] != END_OF_FILE || inDef)
    {
        if (inDef) // Ignore any data inside word definitions
        {
            if (data[i] == END_DEFINE)
            {
                inDef = 0;
            }
            i++;
            continue;
        }
        switch (data[i])
        {
            case DEFINE_ARRAY: {
                int8_t a_bits = data[++i];
                int8_t id1 = data[++i];
                uint16_t a_id = c_to_s(id1, data[++i]);
                int8_t size1 = data[++i];
                uint16_t a_size = c_to_s(size1, data[++i]);
                alloc_array(a_bits, a_id, a_size, &(vm->heap), &next_free);

                // If there is any initialized data, add it to the array
                if (data[++i] == PUSH_LITERAL8 || data[i] == PUSH_LITERAL16 || data[i] == PUSH_LITERAL32)
                {
                    i = init_array_data(a_bits, a_id, data, i, &(vm->heap));
                    break;
                }

                continue;
            }
            case DEFINE_WORD: {
                int8_t id1 = data[++i];
                uint16_t d_id = c_to_s(id1, data[++i]);
                define_word(d_id, &(data[++i]), &(vm->heap), &next_free);
                inDef = 1;
                break;
            }
        }
        i++;
    }

    return initial_ptr;
}

void start_eval(address_t address, VM_Core *vm)
{
    address_t loop_addrs[4];
    int loop_depth = 0;
    while (1)
    {
        int8_t* op = (int8_t*)(vm->heap.mem[address]);
        if (*op == END_DEFINE)
        {
            if (vm->rp > -1)
            {
                address = vm->ret_stack[vm->rp--]; // Get the next address on the return stack
                continue;
            }
            else break;
        }

        switch (*op)
        {
            case CALL:          {
                POP(a);
                vm->ret_stack[++(vm->rp)] = address + 1;
                address = (address_t)a;
                continue;
            }
            case IF:            {
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
            case ELSE:          { while (get_mem(++address) != ENDIF){}; break; }
            case LOOP:          { loop_addrs[++loop_depth] = ++address; continue; }
            case REPEAT:        { address = loop_addrs[loop_depth]; continue; }
            case FOR:           {
                loop_addrs[++loop_depth] = ++address;
                POP(end); vm->loop_maxvals[loop_depth] = end + 1;
                POP(init); vm->loop_ivals[loop_depth] = init;
                continue;
            }
            case NEXT:          {
                if (vm->loop_ivals[loop_depth] == vm->loop_maxvals[loop_depth]) { loop_depth--; break; }
                else { address = loop_addrs[loop_depth]; continue; }
            }
            case ADD_I:         { POP(i); vm->loop_ivals[loop_depth] += i; break; }
            case PUSH_I:        { PUSH(vm->loop_ivals[loop_depth]); break; }
            case BREAK:         {
                int curr_loop_depth = loop_depth;
                while (loop_depth >= curr_loop_depth) // Ignore any possible nested loops
                {
                    int8_t op = get_mem(++address);
                    if (op == FOR || op == LOOP) loop_depth++;
                    else if (op == NEXT || op == REPEAT) loop_depth--;
                }
                break;
            }
            case PUSH_8:        { PUSH(get_mem(++address)); break; }
            case PUSH_16:       {
                int8_t a = get_mem(++address);
                PUSH(c_to_s(a, get_mem(++address)));
                break;
            }
            case PUSH_32:       {
                int8_t a = get_mem(++address);
                int8_t b = get_mem(++address);
                int8_t c = get_mem(++address);
                PUSH(c_to_i(a, b, c, get_mem(++address)));
                break;
            }
            case PUSH_ADDRESS:  { POP(p); PUSH(vm->heap.pointers[p]->address); break; }
            case PUSH_LITERAL8: { int8_t i; while ((i = get_mem(++address)) != END_LITERAL) { PUSH(i); } break; }
            case PUSH_LITERAL16:{ int8_t a; while ((a = get_mem(++address)) != END_LITERAL) { PUSH(c_to_s(a, get_mem(++address))); } break; }
            case PUSH_LITERAL32:{
                int8_t a;
                while ((a = get_mem(++address)) != END_LITERAL)
                {
                    int8_t b = get_mem(++address);
                    int8_t c = get_mem(++address);
                    PUSH(c_to_i(a, b, c, get_mem(++address)));
                }
                break;
            }
            case PUSH_LENGTH:   { POP(p); PUSH((int32_t)(vm->heap.pointers[p]->size)); break; }
            case DUP:           { PUSH(vm->stack[vm->sp-1]); break; }
            case POP:           { vm->sp--; break; }
            case SWAP:          { POP(a); POP(b); PUSH(a); PUSH(b); break; }
            case OVER:          { POP(a); POP(b); PUSH(b); PUSH(a); PUSH(b); break; }
            case ROT:           { POP(a); POP(b); POP(c); PUSH(b); PUSH(a); PUSH(c); break; }
            case ADD:           { POP(a); POP(b); PUSH(a+b); break; }
            case SUB:           { POP(a); POP(b); PUSH(b-a); break; }
            case MUL:           { POP(a); POP(b); PUSH(a*b); break; }
            case DIV_INT:       { POP(a); POP(b); PUSH(b/a); break; }
            case OUT_INT:       { POP(v); printf("%d", v); break; }
            case OUT_CHAR:      { POP(c); printf("%c", c); break; }
            case LOAD:          { POP(a); PUSH(*(int32_t*)(vm->heap.mem[a])); break; }
            case STORE8:        { POP(v); POP(a); *(int8_t*)(vm->heap.mem[a]) = (int8_t)v; break; }
            case STORE16:       { POP(v); POP(a); *(int16_t*)(vm->heap.mem[a]) = (int16_t)v; break; }
            case STORE32:       { POP(v); POP(a); *(int32_t*)(vm->heap.mem[a]) = (int32_t)v; break; }
            case EQUALS:        { POP(a); POP(b); PUSH(a == b); break; }
            case GREATER:       { POP(a); POP(b); PUSH(b > a); break; }
            case LESS:          { POP(a); POP(b); PUSH(b < a); break; }
            case GR_OR_EQ:      { POP(a); POP(b); PUSH(b >= a); break; }
            case LE_OR_EQ:      { POP(a); POP(b); PUSH(b <= a); break; }
            case INVERT:        { POP(a); PUSH(~a); break; }
            case AND:           { POP(a); POP(b); PUSH(a & b); break; }
            case OR:            { POP(a); POP(b); PUSH(a | b); break; }
            case XOR:           { POP(a); POP(b); PUSH(a ^ b); break; }
            case SHIFT_LEFT:    { POP(a); POP(b); PUSH(b << a); break; }
            case SHIFT_RIGHT:   { POP(a); POP(b); PUSH(b >> a); break; }
        }
        address++;
    }
}

void free_mem(VM_Core *vm)
{
    for (unsigned int i = 0; i < vm->heap.num_pointers; i++)
    {
        free(vm->heap.pointers[i]);
    }
    for (unsigned int i = 0; i < vm->heap.mem_size; i++)
    {
        free(vm->heap.mem[i]);
    }
    free(vm->heap.mem);
    free(vm->heap.pointers);
}

void display_mem(VM_Core *vm)
{
    int index = 0;
    printf("#\tP:N\tValue\n");
    for (unsigned int i = 0; i < vm->heap.num_pointers; i++)
    {
        Pointer *p = vm->heap.pointers[i];
        for (unsigned int j = 0; j < p->size; j++)
        {
            if (p->type == ARR8)
            {
                printf("%d\t%d:%d\t%x\n", index, i, j, *(int8_t*)(vm->heap.mem[p->address+j]));
            }
            else if (p->type == ARR16)
            {
                printf("%d\t%d:%d\t%x\n", index, i, j, *(int16_t*)(vm->heap.mem[p->address+j]));
            }
            else if (p->type == ARR32)
            {
                printf("%d\t%d:%d\t%x\n", index, i, j, *(int32_t*)(vm->heap.mem[p->address+j]));
            }

            index++;
        }
        printf("\n");
    }
}

static void alloc_array(uint8_t bits, uint16_t id, uint16_t size, VM_Memory *heap, int *next_free)
{
    (heap->pointers[id])->address = *next_free;
    (heap->pointers[id])->size = size;
    (heap->pointers[id])->type = bits;

    for (int i = 0; i < size; i++)
    {
        if (bits == 8) heap->mem[*next_free + i] = calloc(1, sizeof(int8_t));
        else if (bits == 16) heap->mem[*next_free + i] = calloc(1, sizeof(int16_t));
        else if (bits == 32) heap->mem[*next_free + i] = calloc(1, sizeof(int32_t));
    }

    (*next_free) += size;
}

static void define_word(uint16_t id, int8_t *data, VM_Memory *heap, int *next_free)
{
    int num_words = 0;
    int alloc_words = 20;

    int8_t *words = malloc(sizeof(int8_t) * alloc_words);
    int addressBytes = 0;
    int i = 0;
    while (*(data+i) != END_DEFINE || addressBytes > 0)
    {
        if (*(data+i) == END_DEFINE) break; // TODO: figure out why the code breaks without manually exiting the loop
        if (addressBytes) addressBytes--;

        *(words+i) = *(data+i);
        num_words++;

        if (num_words == alloc_words)
        {
            alloc_words += 20;
            words = realloc(words, sizeof(int8_t) * alloc_words);
        }

        // Ignore the address in instructions that take an address (it might contain 0x00 which is EOF)
        if ((*(data+i) == CALL || *(data+i) == PUSH_ADDRESS) && addressBytes == 0)
        {
            addressBytes = 2;
        }

        i++;
    }

    *(words+i) = *(data+i); // Add END instruction
    num_words++;

    // Allocate memory
    (heap->pointers[id])->address = *next_free;
    (heap->pointers[id])->size = num_words;
    (heap->pointers[id])->type = ARR8;
    for (int i = 0; i < num_words; i++)
    {
        heap->mem[*next_free + i] = calloc(1, sizeof(int8_t));
    }

    // Move instructions into memory
    address_t address = (heap->pointers[id])->address;
    for (i = 0; i < num_words; i++)
    {
        *((int8_t*)heap->mem[address + i]) = *(words + i);
    }

    *next_free += num_words;
    free(words);
}

static int init_array_data(uint8_t bits, uint16_t id, int8_t data[], int i, VM_Memory *heap)
{
    address_t array_addr = (heap->pointers[id])->address;

    if (bits == 8)
    {
        while (data[++i] != END_LITERAL)
        {
            *(int8_t*)heap->mem[array_addr++] = data[i];
        }
    }
    else if (bits == 16)
    {
        int8_t a;
        while ((a = data[++i]) != END_LITERAL)
        {
            *(int16_t*)heap->mem[array_addr++] = c_to_s(a, data[++i]);
        }
    }
    else if (bits == 32)
    {
        int8_t a;
        while ((a = data[++i]) != END_LITERAL)
        {
            int8_t b = data[++i];
            int8_t c = data[++i];
            *(int32_t*)heap->mem[array_addr++] = c_to_i(a, b, c, data[++i]);
        }
    }

    return i;
}

static int16_t c_to_s(int8_t a, int8_t b)
{
    return ((uint8_t)a << 8) | (uint8_t)b;
}

static int32_t c_to_i(int8_t a, int8_t b, int8_t c, int8_t d)
{
    return ((uint8_t)a) << 24 | ((uint8_t)b) << 16 | ((uint8_t)c) << 8 | (uint8_t)d;
}