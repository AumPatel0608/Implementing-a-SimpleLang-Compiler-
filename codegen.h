#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"

// Function declarations for code generator

void init_codegen(void);
void cleanup_codegen(void);
int add_variable(char* name);
int get_variable_address(char* name);
void collect_declarations(ASTNode* node);
int generate_expression_code(ASTNode* node);
void generate_code(ASTNode* node, int depth);

#endif // CODEGEN_H
