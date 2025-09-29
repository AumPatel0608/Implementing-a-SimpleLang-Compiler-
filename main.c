#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

// declare functions from codegen
void init_codegen(void);
void collect_declarations(ASTNode *node);
void generate_code(ASTNode *node, int depth);
void destroy_ast(ASTNode* node);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        printf("Example: %s example.simplelang\n\n", argv[0]);
        printf("SimpleLang Compiler for 8-bit CPU\n");
        printf("==================================\n");
        printf("Supports:\n");
        printf("  - Variable declarations\n");
        printf("  - Arithmetic operations\n");
        printf("  - Conditional statements\n");
        return 1;
    }
    
    FILE *infile = fopen(argv[1], "r");
    if (!infile) {
        printf("Could not open file '%s'\n", argv[1]);
        return 1;
    }
    
    printf("SimpleLang Compiler for 8-bit CPU\n");
    printf("==================================\n");
    printf("Input file: %s\n\n", argv[1]);
    
    // initialize parser
    init_parser(infile);
    
    // parse input
    printf("Parsing SimpleLang source code...\n");
    ASTNode *ast = parse_program();
    printf("Parsing completed successfully!\n\n");
    
    // collect all variable declarations first
    printf("Collecting variable declarations...\n");
    init_codegen();
    collect_declarations(ast);
    
    // print AST structure
    printf("Generated AST:\n");
    print_ast(ast, 0);
    printf("\n");
    
    // generate assembly
    printf("Generating assembly code...\n");
    printf("============================\n");
    
    generate_code(ast, 0);
    
    printf("\nAssembly generation completed!\n");
    
    printf("\nCompiler Statistics:\n");
    printf("===================\n");
    printf("Variables declared: Available in generated assembly\n");
    printf("Memory addresses used: Starting from address 100\n");
    printf("Labels generated: Available in generated assembly\n");
    
    // cleanup
    fclose(infile);
    destroy_ast(ast);
    // cleanup handled in parser
    
    printf("\nCompiler execution completed successfully!\n");
    return 0;
}