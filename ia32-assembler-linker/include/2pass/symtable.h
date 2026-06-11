#ifndef SYMTABLE_H
#define SYMTABLE_H
#include "objfile.h"

#define MAX_SYMBOLS 1024

typedef struct {
    char name[64];
    int address;
    int defined;
    uint8_t is_global;
    uint8_t is_extern;
    uint8_t section_id;
} Symbol;

void symtable_init(void);
int symtable_add(const char* name, uint32_t address, uint8_t section_id, uint8_t is_global, uint8_t is_extern, uint8_t defined);
Symbol* symtable_find(const char* name);
void symtable_print(void);

extern int symbol_count;
extern Symbol symtable[];

extern int rel_count;
extern ObjRelocation reltable[];

void reltable_add(uint32_t offset, const char* symbol_name);

#endif