// main.c
#include "mas.h"
#include <stdio.h>


int main(int argc, char* argv[]) {
    if(argc >2){
        fprintf(stderr, "Too many arguments provided\n");
        return 1;
    }
    else if(argc == 1){
        // REPL mode
        printf("MAS Programming Language REPL \n");
        printf("Type 'exit to quit\n");

        char input[REPL_INPUT_SIZE];

        while(1){
            printf("mas >>");
            if(!(fgets(input, REPL_INPUT_SIZE, stdin))) continue;

            // Check for exit command
            if(strcmp(input, "exit\n") ==0){
                printf("Exiting MAS REPL. Goodbye!\n");
                break;
            }

            lexer_init_repl(input);
            ASTNode* ast = parse_program();
            interpret(ast);
        }
    }
    else{
        //Execution from file
         FILE* f = fopen(argv[1], "rb");
        if (!f) {
            perror("Failed to open file");
            return 1;
        }

        lexer_init(f);
        
        ASTNode* ast = parse_program();
        fclose(f);
        
        interpret(ast);
    }
    
    return 0;
}