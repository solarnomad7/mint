#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interfaces.h"

void ifs_read(VM_Memory *ram);
void ifs_write(VM_Memory *ram, int append);
void ifs_filelen(VM_Memory *ram);
char* ifs_get_path(VM_Memory *ram);

void ifs_update(VM_Memory *ram)
{
    int8_t fn_val = load_mem(PTR_FS_FN, 0, ram);
    switch (fn_val)
    {
        case NONE: break;
        case FS_READ: ifs_read(ram); break;
        case FS_WRITE: ifs_write(ram, 0); break;
        case FS_APPEND: ifs_write(ram, 1); break;
        case FS_FILELEN: ifs_filelen(ram); break;
    }
}

void ifs_read(VM_Memory *ram)
{
    int32_t read_idx = load_mem(PTR_FS_SOURCE, 0, ram);
    int32_t read_len = load_mem(PTR_FS_SOURCE, 1, ram);

    int32_t dest = load_mem(PTR_FS_DEST, 0, ram);

    char *path = ifs_get_path(ram);
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        store_mem(PTR_FS_SOURCE, 0, -1, ram); // Cannot open file
        return;
    }

    int32_t dest_size = ram->pointers[dest].size;

    fseek(file, read_idx, SEEK_SET);
    if (read_len > dest_size)
    {
        // Not enough memory to read the entire file, move the index to the last read byte
        read_len = dest_size;
        store_mem(PTR_FS_SOURCE, 0, read_idx + read_len, ram);
    }
    else
    {
        store_mem(PTR_FS_SOURCE, 0, 0, ram);
    }

    fread(&(ram->mem[ram->pointers[dest].address]), read_len, 1, file);
    fclose(file);
    free(path);

    store_mem(PTR_FS_FN, 0, 0, ram);
}

void ifs_write(VM_Memory *ram, int append)
{
    int32_t source = load_mem(PTR_FS_SOURCE, 0, ram);
    int32_t read_len = load_mem(PTR_FS_SOURCE, 1, ram);

    char *path = ifs_get_path(ram);
    FILE *file;
    file = append ? fopen(path, "ab") : fopen(path, "wb");
    if (!file)
    {
        store_mem(PTR_FS_DEST, 0, -1, ram); // Cannot open file
        return;
    }

    int32_t src_size = ram->pointers[source].size;
    if (read_len > src_size)
    {
        read_len = src_size;
    }

    fwrite(&(ram->mem[ram->pointers[source].address]), read_len, 1, file);
    fclose(file);
    free(path);

    store_mem(PTR_FS_FN, 0, 0, ram);
}

void ifs_filelen(VM_Memory *ram)
{
    char *path = ifs_get_path(ram);
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        store_mem(PTR_FS_DEST, 0, -1, ram); // Cannot open file
        return;
    }

    fseek(file, 0, SEEK_END);
    int length = ftell(file);

    fclose(file);
    free(path);

    store_mem(PTR_FS_SOURCE, 1, (int32_t)length, ram);
    store_mem(PTR_FS_FN, 0, 0, ram);
}

char* ifs_get_path(VM_Memory *ram)
{
    ptrid_t src_path_ptr = load_mem(PTR_FS_PATH, 0, ram);
    address_t src_path_addr = ram->pointers[src_path_ptr].address;

    int path_len = ram->pointers[src_path_ptr].size;
    char *path = malloc(path_len * sizeof(char));
    
    memcpy(path, &(ram->mem[src_path_addr]), path_len);
    path[path_len] = '\0';

    return path;
}