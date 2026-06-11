; test_basico.asm
; Cubre: Directivas DB/DW/DD/RESB, MOV inmediato, Aritmeticas y Logicas

SECTION .data
    var_byte  DB 10
    var_word  DW 1000
    var_dword DD 100000

SECTION .bss
    buffer RESB 64

SECTION .text
GLOBAL _start

_start:
    MOV EAX, 15
    MOV EBX, EAX
    ADD EAX, 5
    SUB EBX, 2
    INC EAX
    DEC EBX
    AND EAX, EBX
    OR ECX, EAX
    XOR EDX, EDX

    INT 0x80