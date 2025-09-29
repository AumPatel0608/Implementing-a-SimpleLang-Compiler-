#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

// AST Node Types
typedef enum {
    AST_DECLARATION,
    AST_ASSIGNMENT,
    AST_BINARY_OP,
    AST_NUMBER,
    AST_IDENTIFIER,
    AST_CONDITIONAL,
    AST_PROGRAM
} ASTType;

// Binary operators
typedef enum {
    OP_ADD,
    OP_SUB,
    OP_EQUAL
} BinaryOperator;

// AST Node structure
typedef struct ASTNode {
    ASTType type;
    union {
        struct {
            char* variable_name;
        } declaration;
        struct {
            char* variable_name;
            struct ASTNode* value;
        } assignment;
        struct {
            BinaryOperator op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;
        struct {
            int value;
        } number;
        struct {
            char* name;
        } identifier;
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_block;
        } conditional;
        struct {
            struct ASTNode** statements;
            int count;
        } program;
    } data;
} ASTNode;

// Function declarations
void init_parser(FILE* file);
ASTNode* parse_program();
ASTNode* parse_statement();
ASTNode* parse_declaration();
ASTNode* parse_assignment();
ASTNode* parse_expression();
ASTNode* parse_conditional();
ASTNode* parse_primary();
ASTNode* create_ast_node(ASTType type);
ASTNode* create_declaration_node(char* variable_name);
ASTNode* create_assignment_node(char* variable_name, ASTNode* value);
ASTNode* create_binary_op_node(BinaryOperator op, ASTNode* left, ASTNode* right);
ASTNode* create_number_node(int value);
ASTNode* create_identifier_node(char* name);
ASTNode* create_conditional_node(ASTNode* condition, ASTNode* then_block);
void free_ast_node(ASTNode* node);
void print_ast(ASTNode* node, int depth);

#endif
