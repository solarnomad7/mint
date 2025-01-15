#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

static char* read_bytes(char* source);

int main(int argc, char **argv)
{
    char *data = read_bytes(argv[1]);

    VM_Core vm;
    ptrid_t initial_ptr = init_vm((int8_t*)data, &vm, argv);
    int result = eval((vm.ram.pointers[initial_ptr]).address, &vm);
    switch (result)
    {
        case SEGMENTATION_FAULT:
            printf("\nMINT: Segmentation fault\n");
            break;
        case MAX_RECURSION_DEPTH:
            printf("\nMINT: Reached maximum recursion depth\n");
            break;
        case STACK_OVERFLOW:
            printf("\nMINT: Stack overflow\n");
            break;
        case STACK_UNDERFLOW:
            printf("\nMINT: Stack underflow\n");
            break;
    }

    free(data);
    free_mem(&vm);
}

static char* read_bytes(char* source)
{
    char *buffer;
    FILE *file;

    int length;
    file = fopen(source, "rb");
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    rewind(file);

    buffer = (char*)malloc(length * sizeof(char));
    fread(buffer, length, 1, file);
    fclose(file);

    return buffer;
}