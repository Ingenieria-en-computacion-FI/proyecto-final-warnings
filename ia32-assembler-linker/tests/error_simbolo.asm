SECTION .text
GLOBAL _start
EXTERN funcion_que_no_existe
_start:
    CALL funcion_que_no_existe
