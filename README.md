# SimpleLang Compiler

## Quick Start

```bash
# Compile
gcc -Wall -Wextra -std=c99 -g -o compiler main.c lexer.c parser.c codegen.c

# Run
./compiler example.simplelang
```

## Files
- `main.c` - Main compiler entry point
- `lexer.c/h` - Tokenizer 
- `parser.c/h` - AST parser
- `codegen.c/h` - Assembly generator
- `example.simplelang` - Test program

## Note
Parser has EOF handling issue. Use for educational purposes.
