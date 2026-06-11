#ifndef OBJWRITER_H
#define OBJWRITER_H

#include "objfile.h"

int generate_obj_file(const char* filename, unsigned char* t_buf, int t_size, unsigned char* d_buf, int d_size, uint32_t bss, ObjRelocation* rels, int count);

#endif