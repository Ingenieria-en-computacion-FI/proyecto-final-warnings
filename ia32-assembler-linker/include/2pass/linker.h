#ifndef LINKER_H
#define LINKER_H

#include <stdint.h>
#include "objfile.h"

#define MAX_INPUT_FILES 10
#define MAX_GLOBAL_SYMBOLS 1024
#define MAX_OUTPUT_SIZE 65536

typedef struct {
    char name[32];
    uint32_t final_address;
    uint32_t section_id;
    uint8_t is_defined;
} GlobalSymbol;

extern GlobalSymbol global_symtable[MAX_GLOBAL_SYMBOLS];
extern int global_symbol_count;

int run_linker(const char* input_filenames[], int file_count, const char* output_filename);

#endif 