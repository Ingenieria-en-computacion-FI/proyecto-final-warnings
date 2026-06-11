SECTION .data
    DB 10
    DW 20

SECTION .text
GLOBAL _start
EXTERN funcion_externa

_start:
    MOV EAX, 10
    MOV EBX, EAX

    MOV EAX, [EBX + ECX * 4 + 8]

    ADD EAX, 5
    CMP EAX, 15
    JE etiqueta_salto

    INC ECX
    JMP fin

etiqueta_salto:
    CALL funcion_externa

fin:
    INT 0x80