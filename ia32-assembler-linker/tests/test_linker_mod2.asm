; test_linker_mod.asm
; Cubre: Módulo secundario y exportación GLOBAL de símbolos

SECTION .text
GLOBAL funcion_externa

funcion_externa:
    ADD EAX, 50
    RET