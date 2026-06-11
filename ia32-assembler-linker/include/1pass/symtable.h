#ifndef SYMTABLE_H
#define SYMTABLE_H

#define MAX_SYMBOLS 1024

typedef struct {
    char name[64];
    int address;
    int defined;  
} Symbol;

void symtable_init(void);
int symtable_add(const char* name, int address, int defined);
Symbol* symtable_find(const char* name);
void symtable_print(void);

#endif