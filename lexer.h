#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

// Token Types
typedef enum {
    TOKEN_INT,           
    TOKEN_IDENTIFIER,    
    TOKEN_NUMBER,        
    TOKEN_ASSIGN,        
    TOKEN_PLUS,          
    TOKEN_MINUS,
    TOKEN_IF,            
    TOKEN_EQUAL,         
    TOKEN_LBRACE,        
    TOKEN_RBRACE,        
    TOKEN_LPAREN,        
    TOKEN_RPAREN,        
    TOKEN_SEMICOLON,     
    TOKEN_UNKNOWN,       
    TOKEN_EOF            
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char text[100];
} Token;

// Function declarations
const char* token_type_to_string(TokenType type);
int getNextToken(FILE* file, Token *token);

#endif
