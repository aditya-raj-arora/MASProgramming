// main.c
#include "mas.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input.mas>\n", argv[0]);
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror("Failed to open file");
        return 1;
    }

    lexer_init(f);

    ASTNode* ast = parse_program();
    fclose(f);
    
    // print_ast(ast, 0); // Print the AST
    MASObject* result = interpret(ast);    
    mas_object_decref(result);
    
    return 0;
}