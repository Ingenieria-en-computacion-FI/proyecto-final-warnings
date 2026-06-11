#ifndef OBJFILE_H
#define OBJFILE_H

#include <stdint.h>

typedef struct {
    char magic[5];          
    uint32_t text_size;     
    uint32_t sym_count;     
} ObjHeader;

typedef struct {
    char name[32];
    uint32_t address;
    uint8_t is_defined;    
} ObjSymbol;

void write_object_file(const char* filename);

#endif