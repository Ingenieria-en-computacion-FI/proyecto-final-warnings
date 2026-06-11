
SECTION .data
    var_8_bits  DB 255         
    var_16_bits DW 65535        
    var_32_bits DD 4000000     

SECTION .bss
    buffer_vacio RESB 128       

SECTION .text
GLOBAL _start

_start:
    MOV EAX, 100
    MOV EBX, 200
    
    ADD EAX, EBX
    SUB EAX, 50
    
    MOV EAX, 1
    MOV EBX, 0
    INT 0x80