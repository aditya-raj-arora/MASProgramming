// main.c
#include "mas.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename.mas>\n", argv[0]);
        return 1;
    }
    
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }
    
    lexer_init(file);
    ASTNode* ast = parse_program();
    fclose(file);
    
    MASObject* result = interpret(ast);
    mas_object_decref(result);
    
    return 0;
}