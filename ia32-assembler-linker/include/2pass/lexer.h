#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_REGISTER,
    TOKEN_INSTRUCTION,
    TOKEN_DIRECTIVE,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_PLUS,
    TOKEN_STAR,
    TOKEN_NEWLINE,
    TOKEN_EOF,
    TOKEN_ERROR,
    TOKEN_STRING
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[64]; 
    int line;        
} Token;

typedef struct {
    const char* source;
    int cursor;
    int line;
    int length;
} Lexer;

void init_lexer(Lexer* lexer, const char* source);
Token* tokenize(Lexer* lexer, int* out_token_count);

#endif