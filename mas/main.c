// main.c
#include "mas.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input.mas>\n", argv[0]);
        return 1;
    }

    FILE* f = fopen(argv[1], "rb");
    if (!f) {
        perror("Failed to open file");
        return 1;
    }

    lexer_init(f);


    
    // Token* tok;
    // do {
    //     tok = lexer_next();
    //     switch (tok->type) {
    //         case TOK_EOF:
    //             printf("EOF\n");
    //             break;
    //         case TOK_ERROR:
    //             printf("ERROR (line %d): %s\n", tok->line, tok->value);
    //             break;
    //         case TOK_NUMBER:
    //             printf("NUMBER (line %d): %s\n", tok->line, tok->value);
    //             break;
    //         case TOK_STRING:
    //             printf("STRING (line %d): \"%s\"\n", tok->line, tok->value);
    //             break;
    //         case TOK_ID:
    //             printf("IDENTIFIER (line %d): %s\n", tok->line, tok->value);
    //             break;
    //         case TOK_NEWLINE:
    //             printf("NEWLINE (line %d)\n", tok->line);
    //             break;
    //         case TOK_PLUS:     printf("PLUS (line %d)\n", tok->line); break;
    //         case TOK_ASSIGN:   printf("ASSIGN '=' (line %d)\n", tok->line); break;
    //         case TOK_EQ:       printf("EQ '==' (line %d)\n", tok->line); break;
    //         // ... add more as needed, or use a helper function ...
    //         default:
    //             // For brevity, just print type and value
    //             printf("TOKEN %d (line %d): %s\n", tok->type, tok->line, tok->value ? tok->value : "");
    //             break;
    //     }

    //     TokenType type = tok->type;

    //     // Clean up
    //     if (tok->value) free(tok->value);
    //     free(tok);

    //     if (type == TOK_EOF) break;
    // }while(1);
      
    ASTNode* ast = parse_program();
    fclose(f);
    
    MASObject* result = interpret(ast);
    mas_object_decref(result);
    
    return 0;
}