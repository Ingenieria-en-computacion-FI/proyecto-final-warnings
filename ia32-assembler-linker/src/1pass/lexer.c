#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/1pass/lexer.h"

void init_lexer(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->cursor = 0;
    lexer->line = 1;
    lexer->length = strlen(source);
}

TokenType classify_lexeme(const char* lexeme) {
    if (strcmp(lexeme, "EAX") == 0 || strcmp(lexeme, "EBX") == 0 || 
        strcmp(lexeme, "ECX") == 0 || strcmp(lexeme, "EDX") == 0 ||
        strcmp(lexeme, "ESI") == 0 || strcmp(lexeme, "EDI") == 0 ||
        strcmp(lexeme, "EBP") == 0 || strcmp(lexeme, "ESP") == 0) {
        return TOKEN_REGISTER;
    }

    if (strcmp(lexeme, "MOV") == 0 || strcmp(lexeme, "PUSH") == 0 || strcmp(lexeme, "POP") == 0 || strcmp(lexeme, "LEA") == 0 ||
        strcmp(lexeme, "ADD") == 0 || strcmp(lexeme, "SUB") == 0 || strcmp(lexeme, "INC") == 0 || strcmp(lexeme, "DEC") == 0 ||
        strcmp(lexeme, "CMP") == 0 || strcmp(lexeme, "NEG") == 0 || strcmp(lexeme, "MUL") == 0 || strcmp(lexeme, "DIV") == 0 ||
        strcmp(lexeme, "IMUL") == 0 || strcmp(lexeme, "IDIV") == 0 ||
        strcmp(lexeme, "AND") == 0 || strcmp(lexeme, "OR") == 0 || strcmp(lexeme, "XOR") == 0 || strcmp(lexeme, "NOT") == 0 ||
        strcmp(lexeme, "JMP") == 0 || strcmp(lexeme, "JE") == 0 || strcmp(lexeme, "JNE") == 0 || strcmp(lexeme, "JG") == 0 ||
        strcmp(lexeme, "JL") == 0 || strcmp(lexeme, "JGE") == 0 || strcmp(lexeme, "JLE") == 0 ||
        strcmp(lexeme, "CALL") == 0 || strcmp(lexeme, "RET") == 0 || strcmp(lexeme, "NOP") == 0 || strcmp(lexeme, "INT") == 0) {
        return TOKEN_INSTRUCTION;
    }

    if (strcmp(lexeme, "SECTION") == 0 || strcmp(lexeme, "GLOBAL") == 0 || strcmp(lexeme, "EXTERN") == 0 ||
        strcmp(lexeme, "DB") == 0 || strcmp(lexeme, "DW") == 0 || strcmp(lexeme, "DD") == 0 ||
        strcmp(lexeme, "RESB") == 0 || strcmp(lexeme, "RESW") == 0 || strcmp(lexeme, "RESD") == 0 ||
        strcmp(lexeme, "ORG") == 0 || strcmp(lexeme, "EQU") == 0) {
        return TOKEN_DIRECTIVE;
    }

    return TOKEN_IDENTIFIER;
}

Token* tokenize(Lexer* lexer, int* out_token_count) {
    int capacity = 100; 
    Token* tokens = malloc(capacity * sizeof(Token));
    int count = 0;

    while (lexer->cursor < lexer->length) {
        char current = lexer->source[lexer->cursor];

        if (current == '\n') {
            lexer->line++;
            lexer->cursor++;
            continue;
        }
        if (isspace(current)) {
            lexer->cursor++;
            continue;
        }

        if (count >= capacity - 1) {
            capacity *= 2;
            tokens = realloc(tokens, capacity * sizeof(Token));
        }

        if (current == ';') {
            while (lexer->cursor < lexer->length && lexer->source[lexer->cursor] != '\n') {
                lexer->cursor++;
            }
            continue;
        }

        if (current == ',') { tokens[count].type = TOKEN_COMMA; strcpy(tokens[count].lexeme, ","); tokens[count].line = lexer->line; count++; lexer->cursor++; continue; }
        if (current == '[') { tokens[count].type = TOKEN_LBRACKET; strcpy(tokens[count].lexeme, "["); tokens[count].line = lexer->line; count++; lexer->cursor++; continue; }
        if (current == ']') { tokens[count].type = TOKEN_RBRACKET; strcpy(tokens[count].lexeme, "]"); tokens[count].line = lexer->line; count++; lexer->cursor++; continue; }
        if (current == '+') { tokens[count].type = TOKEN_PLUS; strcpy(tokens[count].lexeme, "+"); tokens[count].line = lexer->line; count++; lexer->cursor++; continue; }
        if (current == '*') { tokens[count].type = TOKEN_STAR; strcpy(tokens[count].lexeme, "*"); tokens[count].line = lexer->line; count++; lexer->cursor++; continue; }
        if (current == ':') { tokens[count].type = TOKEN_COLON; strcpy(tokens[count].lexeme, ":"); tokens[count].line = lexer->line; count++; lexer->cursor++; continue; }

        if (isalpha(current) || current == '_' || current == '.') {
            int i = 0;
            char buffer[64] = {0};
            while (lexer->cursor < lexer->length && 
                (isalnum(lexer->source[lexer->cursor]) || lexer->source[lexer->cursor] == '_' || lexer->source[lexer->cursor] == '.') && i < 63) {
                
                buffer[i++] = toupper(lexer->source[lexer->cursor++]);
            }
            buffer[i] = '\0';
            
            tokens[count].type = classify_lexeme(buffer);
            strcpy(tokens[count].lexeme, buffer);
            tokens[count].line = lexer->line;
            count++;
            continue;
        }

        if (isdigit(current)) {
            int i = 0;
            char buffer[64] = {0};
            while (lexer->cursor < lexer->length && 
                (isalnum(lexer->source[lexer->cursor])) && i < 63) {
                buffer[i++] = lexer->source[lexer->cursor++];
            }
            buffer[i] = '\0';
            
            tokens[count].type = TOKEN_NUMBER;
            strcpy(tokens[count].lexeme, buffer);
            tokens[count].line = lexer->line;
            count++;
            continue;
        }

        tokens[count].type = TOKEN_ERROR;
        tokens[count].lexeme[0] = current;
        tokens[count].lexeme[1] = '\0';
        tokens[count].line = lexer->line;
        count++;
        lexer->cursor++;
    }

    tokens[count].type = TOKEN_EOF;
    strcpy(tokens[count].lexeme, "EOF");
    tokens[count].line = lexer->line;
    count++;

    *out_token_count = count;
    return tokens;
}
