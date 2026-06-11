#ifndef OBJFILE_H
#define OBJFILE_H

#include <stdint.h>

typedef struct {
    char magic[4];         
    uint32_t text_size;    
    uint32_t data_size;    
    uint32_t bss_size;     
    uint32_t sym_count;     
    uint32_t rel_count;     
} ObjHeader;

typedef struct {
    char name[32];          
    uint32_t address;      
    uint8_t section_id;     
    uint8_t is_global;      
    uint8_t is_extern;      
} ObjSymbol;

typedef struct {
    uint32_t offset;        
    char symbol_name[32];   
} ObjRelocation;

#endif