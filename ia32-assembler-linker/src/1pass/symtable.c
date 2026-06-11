#include <stdio.h>
#include <string.h>
#include "../../include/1pass/symtable.h"

static Symbol table[MAX_SYMBOLS];
static int symbol_count = 0;

void symtable_init(void) {
    symbol_count = 0;
}

int symtable_add(const char* name, int address, int defined) {
    Symbol* existing = symtable_find(name);
    
    if (existing != NULL) {
        if (existing->defined == 0 && defined == 1) {
            existing->address = address;
            existing->defined = 1;
            return 1; 
        }
        return 0;
    }

    if (symbol_count >= MAX_SYMBOLS) {
        return -1; 
    }

    strcpy(table[symbol_count].name, name);
    table[symbol_count].address = address;
    table[symbol_count].defined = defined;
    symbol_count++;
    
    return 1; 
}

Symbol* symtable_find(const char* name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(table[i].name, name) == 0) {
            return &table[i];
        }
    }
    return NULL; 
}

void symtable_print(void) {
    printf("\n--- TABLA DE SIMBOLOS ---\n");
    printf("%-20s | %-12s | %-10s\n", "Nombre", "Direccion", "Definido");
    printf("--------------------------------------------------\n");
    for (int i = 0; i < symbol_count; i++) {
        printf("%-20s | 0x%08X   | %d\n", table[i].name, table[i].address, table[i].defined);
    }
}
