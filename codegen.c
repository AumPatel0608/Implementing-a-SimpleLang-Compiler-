#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef CODEGEN_H
#include "parser.h"

// variable tracking
typedef struct var_entry {
    char *name;
    int addr;
} Variable;

typedef struct cg_state_type {
    Variable vars[26];
    int var_idx;
    int next_addr;
    int label_num;
} CodeGenState;

static CodeGenState cg;

void init_codegen(void) {
    cg.var_idx = 0;
    cg.next_addr = 100;
    cg.label_num = 0;
}

void cleanup_codegen(void) {
    for (int i = 0; i < cg.var_idx; i++) {
        free(cg.vars[i].name);
    }
    cg.var_idx = 0;
}

// add variable to symbol table
int add_variable(char *var_name) {
    // check if already exists
    for (int i = 0; i < cg.var_idx; i++) {
        if (strcmp(cg.vars[i].name, var_name) == 0) {
            return cg.vars[i].addr;
        }
    }
    
    if (cg.var_idx >= 26) {
        printf("Too many variables!\n");
        exit(1);
    }
    
    cg.vars[cg.var_idx].name = (char*)malloc(strlen(var_name) + 1);
    strcpy(cg.vars[cg.var_idx].name, var_name);
    cg.vars[cg.var_idx].addr = cg.next_addr++;
    
    return cg.vars[cg.var_idx++].addr;
}

// get variable address
int get_variable_address(char *var_name) {
    for (int i = 0; i < cg.var_idx; i++) {
        if (strcmp(cg.vars[i].name, var_name) == 0) {
            return cg.vars[i].addr;
        }
    }
    printf("Variable '%s' not found\n", var_name);
    exit(1);
}

// collect all declarations first
void collect_declarations(ASTNode *node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->data.program.count; i++) {
                collect_declarations(node->data.program.statements[i]);
            }
            break;
        case AST_DECLARATION:
            add_variable(node->data.declaration.variable_name);
            break;
        case AST_ASSIGNMENT:
            // implicit declaration for assignments
            add_variable(node->data.assignment.variable_name);
            collect_declarations(node->data.assignment.value);
            break;
        case AST_CONDITIONAL:
            collect_declarations(node->data.conditional.condition);
            collect_declarations(node->data.conditional.then_block);
            break;
        case AST_BINARY_OP:
            collect_declarations(node->data.binary_op.left);
            collect_declarations(node->data.binary_op.right);
            break;
        default:
            break;
    }
}

// generate code for expressions
int gen_expr_code(ASTNode *expr) {
    if (!expr) return 0;
    
    switch (expr->type) {
        case AST_NUMBER:
            printf("ldi A %d\n", expr->data.number.value);
            return 0;
            
        case AST_IDENTIFIER:
            {
                int addr = get_variable_address(expr->data.identifier.name);
                printf("mov A M %d\n", addr);
                return 0;
            }
            
        case AST_BINARY_OP:
            gen_expr_code(expr->data.binary_op.left);
            
            if (expr->data.binary_op.op == OP_EQUAL) {
                // comparison operation
                printf("mov A M %d\n", get_variable_address(expr->data.binary_op.left->data.identifier.name));
                printf("ldi B %d\n", expr->data.binary_op.right->data.number.value);
                printf("cmp\n");
                return 1; // indicates comparison
            } else {
                // addition
                printf("mov B A\n");
                gen_expr_code(expr->data.binary_op.right);
                printf("add\n");
                return 0;
            }
            
        default:
            // handle unsupported AST types
            break;
    }
    return 0;
}

// generate assembly code
void generate_code(ASTNode *node, int depth) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM:
            printf(".text\n");
            for (int i = 0; i < node->data.program.count; i++) {
                generate_code(node->data.program.statements[i], depth);
            }
            
            printf("\n.data\n");
            for (int i = 0; i < cg.var_idx; i++) {
                printf("%s_addr = %d\n", cg.vars[i].name, cg.vars[i].addr);
            }
            printf("hlt\n");
            break;
            
        case AST_DECLARATION:
            // declarations handled in collect_declarations
            break;
            
        case AST_ASSIGNMENT:
            printf("; %s = ...\n", node->data.assignment.variable_name);
            gen_expr_code(node->data.assignment.value);
            int addr = get_variable_address(node->data.assignment.variable_name);
            printf("mov M A %d\n", addr);
            break;
            
        case AST_CONDITIONAL:
            {
                printf("; if (condition) {\n");
                
                gen_expr_code(node->data.conditional.condition);
                
                int else_lbl = cg.label_num++;
                int end_lbl = cg.label_num++;
                
                printf("jnz else_%d\n", else_lbl);
                printf(".text\n");
                
                generate_code(node->data.conditional.then_block, depth + 1);
                
                printf("\n.data\n");
                for (int i = 0; i < cg.var_idx; i++) {
                    printf("%s_addr = %d\n", cg.vars[i].name, cg.vars[i].addr);
                }
                printf("jmp end_%d\n", end_lbl);
                printf("else_%d:\n", else_lbl);
                printf("end_%d:\n", end_lbl);
                printf("hlt\n");
                
                printf("\n.data\n");
                for (int i = 0; i < cg.var_idx; i++) {
                    printf("%s_addr = %d\n", cg.vars[i].name, cg.vars[i].addr);
                }
            }
            break;
            
        default:
            printf("Unknown node type for code generation\n");
            exit(1);
    }
}

#endif