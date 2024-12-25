#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

static char* read_bytes(char* source);

int main(int argc, char **argv)
{
    char *data = read_bytes(argv[1]);

    VM_Core vm;
    uint16_t initial_ptr = init_vm((int8_t*)data, &vm);
    start_eval((vm.heap.pointers[initial_ptr])->address, &vm);

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