#include <stdio.h>
#include <string.h>
#include "../../include/2pass/symtable.h"

Symbol symtable[MAX_SYMBOLS]; 
int symbol_count = 0;

int rel_count = 0;
ObjRelocation reltable[256];

void symtable_init(void) {
    symbol_count = 0;
}

int symtable_add(const char* name, uint32_t address, uint8_t section_id, uint8_t is_global, uint8_t is_extern, uint8_t defined) {
    Symbol* existing = symtable_find(name);
    
    if (existing != NULL) {
        if (existing->defined == 0 && defined == 1) {
            existing->address = address;
            existing->defined = 1;
            existing->section_id = section_id;
        }
        return 0;
    }

    if (symbol_count >= MAX_SYMBOLS) {
        return -1;
    }

    strcpy(symtable[symbol_count].name, name);
    symtable[symbol_count].address = address;
    symtable[symbol_count].defined = defined;
    symtable[symbol_count].is_global  = is_global;
    symtable[symbol_count].is_extern  = is_extern;
    symtable[symbol_count].section_id = section_id;
    symbol_count++;
    return 1; 
}

Symbol* symtable_find(const char* name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symtable[i].name, name) == 0) {
            return &symtable[i];
        }
    }
    return NULL;
}

void symtable_print(void) {
    printf("\n--- TABLA DE SIMBOLOS ---\n");
    printf("%-20s | %-12s | %-10s\n", "Nombre", "Direccion", "Definido");
    printf("--------------------------------------------------\n");
    for (int i = 0; i < symbol_count; i++) {
        printf("%-20s | 0x%08X   | %d\n", symtable[i].name, symtable[i].address, symtable[i].defined);
    }
}

void reltable_add(uint32_t offset, const char* symbol_name) {
    if (rel_count < 256) {
        reltable[rel_count].offset = offset;
        strncpy(reltable[rel_count].symbol_name, symbol_name, 31);
        rel_count++;
    }
}