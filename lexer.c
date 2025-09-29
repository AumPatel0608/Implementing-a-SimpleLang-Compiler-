// Simple lexer for SimpleLang
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

#define MAX_TOKEN_LEN 100

// Token parsing function
int getNextToken(FILE *f, Token *t) {
    if (f == NULL) {
        t->type = TOKEN_EOF;
        return 0;
    }

    int chr;
    
    // skip spaces and tabs
    while ((chr = fgetc(f)) != EOF) {
        if (!isspace(chr)) {
            break;
        }
    }
    
    if (chr == EOF) {
        t->type = TOKEN_EOF;
        strcpy(t->text, "");
        return 0;
    }
    
    // handle letters and keywords
    if (isalpha(chr)) {
        int i = 0;
        t->text[i++] = chr;
        
        while ((chr = fgetc(f)) != EOF && isalnum(chr)) {
            if (i < MAX_TOKEN_LEN - 1) {
                t->text[i++] = chr;
            }
        }
        
        if (chr != EOF) ungetc(chr, f);
        t->text[i] = '\0';
        
        // check if keywords
        if (strcmp(t->text, "int") == 0) {
            t->type = TOKEN_INT;
        } else if (strcmp(t->text, "if") == 0) {
            t->type = TOKEN_IF;
        } else {
            t->type = TOKEN_IDENTIFIER;
        }
        return 1;
    }
    
    // handle numbers
    if (isdigit(chr)) {
        int i = 0;
        t->text[i++] = chr;
        
        while ((chr = fgetc(f)) != EOF && isdigit(chr)) {
            if (i < MAX_TOKEN_LEN - 1) {
                t->text[i++] = chr;
            }
        }
        
        if (chr != EOF) ungetc(chr, f);
        t->text[i] = '\0';
        t->type = TOKEN_NUMBER;
        return 1;
    }
    
    // handle operators and symbols
    switch (chr) {
        case '=':
            chr = fgetc(f);
            if (chr == '=') {
                t->type = TOKEN_EQUAL;
                strcpy(t->text, "==");
            } else {
                if (chr != EOF) ungetc(chr, f);
                t->type = TOKEN_ASSIGN;
                strcpy(t->text, "=");
            }
            return 1;
            
        case '+':
            t->type = TOKEN_PLUS;
            strcpy(t->text, "+");
            return 1;
            
        case '-':
            t->type = TOKEN_MINUS;
            strcpy(t->text, "-");
            return 1;
            
        case '{':
            t->type = TOKEN_LBRACE;
            strcpy(t->text, "{");
            return 1;
            
        case '}':
            t->type = TOKEN_RBRACE;
            strcpy(t->text, "}");
            return 1;
            
        case '(':
            t->type = TOKEN_LPAREN;
            strcpy(t->text, "(");
            return 1;
            
        case ')':
            t->type = TOKEN_RPAREN;
            strcpy(t->text, ")");
            return 1;
            
        case ';':
            t->type = TOKEN_SEMICOLON;
            strcpy(t->text, ";");
            return 1;
            
        case '/':
            chr = fgetc(f);
            if (chr == '/') {
                // skip comment line
                while ((chr = fgetc(f)) != EOF && chr != '\n');
                return getNextToken(f, t);
            } else {
                if (chr != EOF) ungetc(chr, f);
                t->type = TOKEN_UNKNOWN;
                strcpy(t->text, "/");
                return 1;
            }
            
        default:
            t->type = TOKEN_UNKNOWN;
            t->text[0] = chr;
            t->text[1] = '\0';
            return 1;
    }
}

// convert token type to readable string
const char* get_token_string(TokenType tt) {
    switch (tt) {
        case TOKEN_INT: return "KEYWORD_INT";
        case TOKEN_IF: return "KEYWORD_IF";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_EQUAL: return "EQUAL";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_UNKNOWN: return "UNKNOWN";
        case TOKEN_EOF: return "EOF";
        default: return "UNKNOWN";
    }
}