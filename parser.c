#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

// global state for parsing
static FILE *parsing_file = NULL;
static Token curr_token;
static int token_available = 0;

// helper for token getNextToken call
int getNextToken_impl(FILE *f, Token *t) {
    (void)parsing_file; // suppress unused warning
    return getNextToken(f, t);
}

// advance to next token
void advance_token() {
    if (parsing_file) {
        token_available = getNextToken(parsing_file, &curr_token);
    } else {
        curr_token.type = TOKEN_EOF;
        strcpy(curr_token.text, "");
        token_available = 0;
    }
}

// expect specific token type
void require_token(TokenType expected) {
    if (!token_available) {
        advance_token();
    }
    if (curr_token.type != expected) {
        printf("Parse Error: Expected %d, got %d ('%s')\n", 
               expected, curr_token.type, curr_token.text);
        exit(1);
    }
    token_available = 0;
}

// create new AST node
ASTNode* new_ast_node(ASTType type) {
    ASTNode* n = (ASTNode*)malloc(sizeof(ASTNode));
    if (!n) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    n->type = type;
    memset(&n->data, 0, sizeof(n->data));
    return n;
}

// create declaration node
ASTNode* make_decl_node(char* var_name) {
    ASTNode* n = new_ast_node(AST_DECLARATION);
    n->data.declaration.variable_name = (char*)malloc(strlen(var_name) + 1);
    if (!n->data.declaration.variable_name) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    strcpy(n->data.declaration.variable_name, var_name);
    return n;
}

// create assignment node
ASTNode* make_assign_node(char* var_name, ASTNode* val) {
    ASTNode* n = new_ast_node(AST_ASSIGNMENT);
    n->data.assignment.variable_name = (char*)malloc(strlen(var_name) + 1);
    if (!n->data.assignment.variable_name) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    strcpy(n->data.assignment.variable_name, var_name);
    n->data.assignment.value = val;
    return n;
}

// create binary operation node
ASTNode* make_binop_node(BinaryOperator op, ASTNode* left, ASTNode* right) {
    ASTNode* n = new_ast_node(AST_BINARY_OP);
    n->data.binary_op.op = op;
    n->data.binary_op.left = left;
    n->data.binary_op.right = right;
    return n;
}

// create number node
ASTNode* make_num_node(int val) {
    ASTNode* n = new_ast_node(AST_NUMBER);
    n->data.number.value = val;
    return n;
}

// create identifier node
ASTNode* make_id_node(char* name) {
    ASTNode* n = new_ast_node(AST_IDENTIFIER);
    n->data.identifier.name = (char*)malloc(strlen(name) + 1);
    if (!n->data.identifier.name) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    strcpy(n->data.identifier.name, name);
    return n;
}

// create conditional node
ASTNode* make_if_node(ASTNode* cond, ASTNode* body) {
    ASTNode* n = new_ast_node(AST_CONDITIONAL);
    n->data.conditional.condition = cond;
    n->data.conditional.then_block = body;
    return n;
}

// destroy AST node and children
void destroy_ast(ASTNode* n) {
    if (!n) return;
    
    switch (n->type) {
        case AST_DECLARATION:
            free(n->data.declaration.variable_name);
            break;
        case AST_ASSIGNMENT:
            free(n->data.assignment.variable_name);
            destroy_ast(n->data.assignment.value);
            break;
        case AST_BINARY_OP:
            destroy_ast(n->data.binary_op.left);
            destroy_ast(n->data.binary_op.right);
            break;
        case AST_IDENTIFIER:
            free(n->data.identifier.name);
            break;
        case AST_CONDITIONAL:
            destroy_ast(n->data.conditional.condition);
            destroy_ast(n->data.conditional.then_block);
            break;
        case AST_PROGRAM:
            for (int i = 0; i < n->data.program.count; i++) {
                destroy_ast(n->data.program.statements[i]);
            }
            free(n->data.program.statements);
            break;
        default:
            break;
    }
    free(n);
}

// check if current token matches
int check_token(TokenType t) {
    if (!token_available) {
        advance_token();
    }
    return curr_token.type == t;
}

// parse identifier or number
ASTNode* parse_term() {
    if (!check_token(TOKEN_NUMBER) && !check_token(TOKEN_IDENTIFIER)) {
        advance_token();
    }
    
    if (curr_token.type == TOKEN_NUMBER) {
        int num = atoi(curr_token.text);
        token_available = 0;
        return make_num_node(num);
    }
    
    if (curr_token.type == TOKEN_IDENTIFIER) {
        char* id_name = (char*)malloc(strlen(curr_token.text) + 1);
        strcpy(id_name, curr_token.text);
        token_available = 0;
        ASTNode* node = make_id_node(id_name);
        free(id_name);
        return node;
    }
    
    printf("Syntax Error: Expected number or identifier, got '%s'\n", curr_token.text);
    return NULL;
}

// parse expression with operators
ASTNode* parse_expr() {
    ASTNode* left = parse_term();
    
    if (!token_available) {
        advance_token();
    }
    
    while (curr_token.type == TOKEN_PLUS || curr_token.type == TOKEN_MINUS) {
        BinaryOperator op;
        if (curr_token.type == TOKEN_PLUS) {
            op = OP_ADD;
        } else {
            op = OP_SUB;
        }
        token_available = 0;
        advance_token();
        ASTNode* right = parse_term();
        left = make_binop_node(op, left, right);
    }
    
    return left;
}

// parse equality comparison
ASTNode* parse_comparison() {
    ASTNode* left = parse_expr();
    
    if (!token_available) {
        advance_token();
    }
    
    if (curr_token.type == TOKEN_EQUAL) {
        token_available = 0;
        ASTNode* right = parse_expr();
        return make_binop_node(OP_EQUAL, left, right);
    }
    
    return left;
}

// parse variable declaration
ASTNode* parse_decl_stmt() {
    require_token(TOKEN_INT);
    advance_token(); // move to identifier
    char var_name[100];
    strcpy(var_name, curr_token.text);
    require_token(TOKEN_IDENTIFIER);
    advance_token(); // move past semicolon
    require_token(TOKEN_SEMICOLON);
    return make_decl_node(var_name);
}

// parse assignment statement
ASTNode* parse_assign_stmt() {
    char var_name[100];
    strcpy(var_name, curr_token.text);
    token_available = 0;
    require_token(TOKEN_ASSIGN);
    ASTNode* val = parse_comparison();
    require_token(TOKEN_SEMICOLON);
    return make_assign_node(var_name, val);
}

// parse conditional statement
ASTNode* parse_if_stmt() {
    require_token(TOKEN_IF);
    require_token(TOKEN_LPAREN);
    advance_token(); // get identifier in condition
    ASTNode* cond = parse_comparison();
    require_token(TOKEN_RPAREN);
    require_token(TOKEN_LBRACE);
    
    ASTNode* body = parse_program();
    
    require_token(TOKEN_RBRACE);
    return make_if_node(cond, body);
}

// parse single statement
ASTNode* parse_stmt() {
    if (!check_token(TOKEN_INT) && !check_token(TOKEN_IDENTIFIER) && !check_token(TOKEN_IF)) {
        advance_token();
    }
    
    if (curr_token.type == TOKEN_INT) {
        return parse_decl_stmt();
    } else if (curr_token.type == TOKEN_IDENTIFIER) {
        return parse_assign_stmt();
    } else if (curr_token.type == TOKEN_IF) {
        return parse_if_stmt();
    } else if (curr_token.type == TOKEN_EOF) {
        return NULL; // end of file
    }
    
    printf("Parse Error: Unexpected token '%s' (type %d)\n", curr_token.text, curr_token.type);
    exit(1);
    return NULL;
}

// parse entire program
ASTNode* parse_program() {
    ASTNode* prog = new_ast_node(AST_PROGRAM);
    prog->data.program.statements = (ASTNode**)malloc(sizeof(ASTNode*) * 100);
    prog->data.program.count = 0;
    
    advance_token();
    
    while (curr_token.type != TOKEN_EOF) {
        ASTNode* stmt = parse_stmt();
        if (stmt) {
            prog->data.program.statements[prog->data.program.count++] = stmt;
        }
        if (curr_token.type != TOKEN_EOF) {
            advance_token();
        } else {
            break;
        }
    }
    
    return prog;
}

// initialize parser
void init_parser(FILE* f) {
    parsing_file = f;
    token_available = 0;
}

// print AST for debugging
void print_ast(ASTNode* n, int indent) {
    if (!n) return;
    
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    
    switch (n->type) {
        case AST_PROGRAM:
            printf("PROGRAM (%d statements)\n", n->data.program.count);
            for (int i = 0; i < n->data.program.count; i++) {
                print_ast(n->data.program.statements[i], indent + 1);
            }
            break;
        case AST_DECLARATION:
            printf("DECLARATION: %s\n", n->data.declaration.variable_name);
            break;
        case AST_ASSIGNMENT:
            printf("ASSIGNMENT: %s =\n", n->data.assignment.variable_name);
            print_ast(n->data.assignment.value, indent + 1);
            break;
        case AST_BINARY_OP:
            printf("BINARY: %d\n", n->data.binary_op.op);
            print_ast(n->data.binary_op.left, indent + 1);
            print_ast(n->data.binary_op.right, indent + 1);
            break;
        case AST_NUMBER:
            printf("NUMBER: %d\n", n->data.number.value);
            break;
        case AST_IDENTIFIER:
           printf("IDENTIFIER: %s\n", n->data.identifier.name);
            break;
        case AST_CONDITIONAL:
            printf("IF\n");
            print_ast(n->data.conditional.condition, indent + 1);
            print_ast(n->data.conditional.then_block, indent + 1);
            break;
    }
}