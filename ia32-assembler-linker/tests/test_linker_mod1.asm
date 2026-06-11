; test_linker_mod1.asm
; Cubre: todas las instrucciones, directivas, modos de direccionamiento

STDIN:   EQU 0
STDOUT:  EQU 1
SYSCALL: EQU 0x80

SECTION .data
    DB 10
    DW 2000
    DD 99999
    DB "hola mundo"
mensaje:
    DB "test"

SECTION .bss
buffer:
    RESB 64
    RESW 4
    RESD 2

SECTION .text
GLOBAL _start
EXTERN funcion_externa

_start:
    MOV EAX, 10
    MOV EBX, STDOUT

    MOV ECX, EAX

    MOV EAX, [1000]

    MOV EAX, [EBX+ECX*4+8]

    ADD EAX, 5
    SUB EBX, 3
    INC ECX
    DEC EDX
    CMP EAX, 15

    AND EAX, EBX
    OR  EBX, ECX
    XOR ECX, ECX
    NOT EAX
    NEG EBX

    JE  igual
    JNE diferente
    JG  mayor
    JL  menor
    JGE mayor_igual
    JLE menor_igual

    JMP fin

igual:
    INC EAX
    JMP fin

diferente:
    DEC EAX
    JMP fin

mayor:
    ADD EAX, 1
    JMP fin

menor:
    SUB EAX, 1
    JMP fin

mayor_igual:
    MOV EBX, EAX
    JMP fin

menor_igual:
    MOV ECX, EAX
    JMP fin

    CALL funcion_externa

    PUSH EAX
    PUSH 42
    POP  EBX

    LEA EAX, [EBX+ECX*2]

    MUL EBX
    DIV ECX

    NOP
    RET

fin:
    INT SYSCALL