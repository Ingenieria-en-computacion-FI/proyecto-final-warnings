#include <stdio.h>
#include "../../include/1pass/objfile.h"
#include "../../include/1pass/symtable.h"

extern unsigned char machine_code_buffer[8192];
extern int machine_code_size;

void write_object_file(const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Error: No se pudo crear el archivo '%s'\n", filename);
        return;
    }

    ObjHeader header = {"UNAM", machine_code_size, 0};
    
    fwrite(&header, sizeof(ObjHeader), 1, f);
    
    fwrite(machine_code_buffer, 1, machine_code_size, f);
    

    
    fclose(f);
    printf("Archivo objeto '%s' generado exitosamente (%d bytes).\n", filename, machine_code_size);
}
