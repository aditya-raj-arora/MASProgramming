// parser.c
#include "mas.h"

static Token* current_token = NULL; // The token we are currently looking at

static void advance() {
    // Free the old token if it exists
    if (current_token && current_token->type != TOK_EOF) {
        if (current_token->value) free(current_token->value);
        free(current_token);
    }
    current_token = lexer_next();
}

static bool match(TokenType type) {
    if (current_token && current_token->type == type) {
        return true;
    }
    return false;
}

static void consume(TokenType type, const char* message) {
    if (!match(type)) {
        fprintf(stderr, "Parse error at line %d: %s\n", 
                current_token ? current_token->line : -1, message);
        exit(1);
    }
    advance();
}

// Forward declarations for recursive parsing
ASTNode* parse_statement();
ASTNode* parse_expression();
ASTNode* parse_comparison();
ASTNode* parse_term();
ASTNode* parse_factor();
ASTNode* parse_unary();
ASTNode* parse_primary();

// Parse program
ASTNode* parse_program() {
    ASTNode* program = malloc(sizeof(ASTNode));
    program->type = AST_PROGRAM;
    program->line = 1;
    
    // Parse statements until EOF
    int stmt_count = 0;
    ASTNode** statements = malloc(sizeof(ASTNode*) * 100); // TODO: dynamic resize
    
    advance(); // get first token
    while (current_token && current_token->type != TOK_EOF) {
        if (current_token->type == TOK_NEWLINE) {
            advance();
            continue;
        }
        statements[stmt_count++] = parse_statement();

        // After a statement, we must have a newline or EOF.
        if (current_token->type == TOK_EOF ) {
            advance();
        }
        else if (current_token->type != TOK_EOF ) {
            consume(TOK_NEWLINE, "Expected newline after statement");
        }
    }
    
    program->data.list.items = statements;
    program->data.list.count = stmt_count;
    return program;
}

// Parse statement
ASTNode* parse_statement() {
    int start_line = current_token->line;
    if (match(KW_DEF)) {
    advance(); // consume 'def'
    
    if (!match(TOK_ID)) {
        fprintf(stderr, "Parse error at line %d: Expected function name\n", current_token->line);
        exit(1);
    }
    char* func_name = strdup(current_token->value);
    advance(); // consume function name

    consume(TOK_LPAREN, "Expected '('");
    
    char** params = malloc(sizeof(char*) * 10);
    int param_count = 0;
    
    if (!match(TOK_RPAREN)) {
        do {
            if (!match(TOK_ID)) {
                fprintf(stderr, "Parse error at line %d: Expected parameter name\n", current_token->line);
                exit(1);
            }
            params[param_count++] = strdup(current_token->value);
            advance(); // consume parameter name
        } while (match(TOK_COMMA) && (advance(), 1)); // consume comma
    }
    
    consume(TOK_RPAREN, "Expected ')'");
    consume(TOK_COLON, "Expected ':'");
    consume(TOK_NEWLINE, "Expected newline after function header");
    
    // Parse function body
    int body_count = 0;
    ASTNode** body = malloc(sizeof(ASTNode*) * 100);
    while (current_token && current_token->type != TOK_END) {
        if (current_token->type == TOK_NEWLINE) {
            advance();
            continue;
        }
        body[body_count++] = parse_statement();
    }
    consume(TOK_END, "Expected 'end' to close function");
    
    ASTNode* func = malloc(sizeof(ASTNode));
    func->type = AST_FUNCDEF;
    func->line = start_line;
    func->data.funcdef.name = func_name;
    func->data.funcdef.params = params;
    func->data.funcdef.param_count = param_count;
    func->data.funcdef.body = body;
    func->data.funcdef.body_count = body_count;
    return func;
}
    else if (match(KW_LOOP)) {
        int loop_line = current_token->line;
        advance(); // consume 'loop'
        ASTNode* condition = parse_expression();
        consume(TOK_COLON, "Expected ':'");
        consume(TOK_NEWLINE, "Expected newline after loop condition");
        
        int body_count = 0;
        ASTNode** body = malloc(sizeof(ASTNode*) * 100);
        while (current_token && current_token->type != TOK_END) {
            if (current_token->type == TOK_NEWLINE) {
                advance();
                continue;
            }
            body[body_count++] = parse_statement();
        }
        consume(TOK_END, "Expected 'end' to close loop");
        
        ASTNode* loop = malloc(sizeof(ASTNode));
        loop->type = AST_LOOP;
        loop->line = loop_line;
        loop->data.loop.condition = condition;
        loop->data.loop.body = body;
        loop->data.loop.body_count = body_count;
        return loop;
    }
    else if (match(KW_EACH)) {
        int each_line = current_token->line;
        advance(); // consume 'each'
    
    if (!match(TOK_ID)) {
        fprintf(stderr, "Parse error at line %d: Expected variable name\n", current_token->line);
        exit(1);
    }
    char* target = strdup(current_token->value);
    advance(); // consume identifier

    consume(KW_IN, "Expected 'in'");

    // Check if it's a range: <expr> to <expr>
    ASTNode* range_start = NULL;
    ASTNode* range_end = NULL;
    ASTNode* iterable = NULL;

    // Peek ahead: if we see "to" after an expression, it's a range
    // Parse the first expression
    ASTNode* first = parse_expression();
    
    if (match(KW_TO)) {
        // It's a range: first = start, parse end
        advance(); // consume 'to'
        range_start = first;
        range_end = parse_expression();
    } else {
        // It's a normal iterable (list, etc.)
        iterable = first;
    }

    consume(TOK_COLON, "Expected ':'");
    consume(TOK_NEWLINE, "Expected newline after each header");
    
    // Parse body
    int body_count = 0;
    ASTNode** body = malloc(sizeof(ASTNode*) * 100);
    while (current_token && current_token->type != TOK_END) {
        if (current_token->type == TOK_NEWLINE) {
            advance();
            continue;
        }
        body[body_count++] = parse_statement();
    }
    consume(TOK_END, "Expected 'end' to close each");
    
    ASTNode* each = malloc(sizeof(ASTNode));
    each->type = AST_EACH;
    each->line = each_line;
    each->data.each.target = target;
    each->data.each.iterable = iterable;      // NULL for ranges
    each->data.each.range_start = range_start; // NULL for iterables
    each->data.each.range_end = range_end;
    each->data.each.body = body;
    each->data.each.body_count = body_count;
    return each;
}
else if (match(KW_IF)) {
    int if_line = current_token->line;
    advance(); // consume 'if'
    ASTNode* condition = parse_expression();
    consume(TOK_COLON, "Expected ':'");
    consume(TOK_NEWLINE, "Expected newline after if condition");
    
    // Parse 'then' body (stop at 'else' or 'end')
    int then_body_count = 0;
    ASTNode** then_body = malloc(sizeof(ASTNode*) * 100);
    while (current_token && current_token->type != TOK_END && current_token->type != KW_ELSE) {
        if (current_token->type == TOK_NEWLINE) {
            advance();
            continue;
        }
        then_body[then_body_count++] = parse_statement();
    }

    // Parse optional 'else' block
    ASTNode** else_body = NULL;
    int else_body_count = 0;
    if (match(KW_ELSE)) {
        advance(); // consume 'else'
        consume(TOK_COLON, "Expected ':' after else");
        consume(TOK_NEWLINE, "Expected newline after else");

        else_body = malloc(sizeof(ASTNode*) * 100);
        while (current_token && current_token->type != TOK_END) {
            if (current_token->type == TOK_NEWLINE) {
                advance();
                continue;
            }
            else_body[else_body_count++] = parse_statement();
        }
    }

    consume(TOK_END, "Expected 'end' to close if");

    ASTNode* if_node = malloc(sizeof(ASTNode));
    if_node->type = AST_IF;
    if_node->line = if_line;
    if_node->data.if_stmt.condition = condition;
    if_node->data.if_stmt.then_body = then_body;
    if_node->data.if_stmt.then_body_count = then_body_count;
    if_node->data.if_stmt.else_body = else_body;          // NULL if no else
    if_node->data.if_stmt.else_body_count = else_body_count;
    return if_node;
}
    else if (match(KW_GIVE)) {
        int give_line = current_token->line;
        advance(); // consume 'give'
        ASTNode* value = parse_expression();
        ASTNode* ret = malloc(sizeof(ASTNode));
        ret->type = AST_RETURN;
        ret->line = give_line;
        ret->data.expr = value;
        return ret;
    }
    else if (match(KW_STOP)) {
        int stop_line = current_token->line;
        advance(); // consume 'stop'
        ASTNode* brk = malloc(sizeof(ASTNode));
        brk->type = AST_BREAK;
        brk->line = stop_line;
        return brk;
    }
    else if (match(KW_NEXT)) {
        int next_line = current_token->line;
        advance(); // consume 'next'
        ASTNode* cont = malloc(sizeof(ASTNode));
        cont->type = AST_CONTINUE;
        cont->line = next_line;
        return cont;
    }
    else if (match(KW_PRINT)) {
        int print_line = current_token->line;
        advance(); // consume 'print'
        // Handle print as a function call expression
        ASTNode** args = malloc(sizeof(ASTNode*) * 10); // Allow multiple args
        int arg_count = 0;

        // In many languages, print can take a list of comma-separated expressions
        do {
            args[arg_count++] = parse_expression();
            if (match(TOK_COMMA)) {
                advance(); // consume comma
            } else break;
        } while (true);

        ASTNode* call = malloc(sizeof(ASTNode));
        call->type = AST_CALL;
        call->line = current_token ? print_line : -1;
        call->data.call.name = strdup("print"); // The name of the built-in
        call->data.call.args = args;
        call->data.call.arg_count = arg_count;

        // Wrap it in an expression statement
        ASTNode* stmt = malloc(sizeof(ASTNode));
        stmt->type = AST_EXPRSTMT;
        stmt->line = call->line;
        stmt->data.expr = call;
        return stmt;
    } else {
        // If it's not a keyword-led statement, it must be an expression statement.
        ASTNode* expr = parse_expression();
        ASTNode* stmt = malloc(sizeof(ASTNode));
        stmt->type = AST_EXPRSTMT;
        stmt->line = expr->line;
        stmt->data.expr = expr;
        return stmt;
    }
}

// Parse expression (simplified - left associative)
ASTNode* parse_expression() {
    return parse_comparison();
}

ASTNode* parse_comparison() {
    ASTNode* expr = parse_term(); // Parse the left-hand side

    // Check for assignment, which has the lowest precedence
    if (match(TOK_ASSIGN)) {
        advance(); // consume '='
        if (expr->type != AST_VAR) {
            fprintf(stderr, "Parse error at line %d: Invalid assignment target.\n", expr->line);
            exit(1);
        }
        ASTNode* value = parse_expression();
        ASTNode* assign = malloc(sizeof(ASTNode));
        assign->type = AST_ASSIGN;
        assign->line = expr->line;
        assign->data.assign.name = expr->data.var_name;
        assign->data.assign.value = value;
        return assign;
    }
    
    while (current_token) {
        if (match(TOK_EQ)) {
            advance();
            ASTNode* right = parse_term();
            ASTNode* binop = malloc(sizeof(ASTNode));
            binop->type = AST_BINOP;
            binop->line = current_token->line;
            binop->data.binop.left = expr;
            binop->data.binop.op = strdup("==");
            binop->data.binop.right = right;
            expr = binop;
        }
        else if (match(TOK_NEQ)) {
            advance();
            ASTNode* right = parse_term();
            ASTNode* binop = malloc(sizeof(ASTNode));
            binop->type = AST_BINOP;
            binop->line = current_token->line;
            binop->data.binop.left = expr;
            binop->data.binop.op = strdup("!=");
            binop->data.binop.right = right;
            expr = binop;
        }
        else if (match(TOK_LT)) {
            advance();
            ASTNode* right = parse_term();
            ASTNode* binop = malloc(sizeof(ASTNode));
            binop->type = AST_BINOP;
            binop->line = current_token->line;
            binop->data.binop.left = expr;
            binop->data.binop.op = strdup("<");
            binop->data.binop.right = right;
            expr = binop;
        }
        else if (match(TOK_LE)) {
            advance();
            ASTNode* right = parse_term();
            ASTNode* binop = malloc(sizeof(ASTNode));
            binop->type = AST_BINOP;
            binop->line = current_token->line;
            binop->data.binop.left = expr;
            binop->data.binop.op = strdup("<=");
            binop->data.binop.right = right;
            expr = binop;
        }
        else if (match(TOK_GT)) {
            advance();
            ASTNode* right = parse_term();
            ASTNode* binop = malloc(sizeof(ASTNode));
            binop->type = AST_BINOP;
            binop->line = current_token->line;
            binop->data.binop.left = expr;
            binop->data.binop.op = strdup(">");
            binop->data.binop.right = right;
            expr = binop;
        }
        else if (match(TOK_GE)) {
            advance();
            ASTNode* right = parse_term();
            ASTNode* binop = malloc(sizeof(ASTNode));
            binop->type = AST_BINOP;
            binop->line = current_token->line;
            binop->data.binop.left = expr;
            binop->data.binop.op = strdup(">=");
            binop->data.binop.right = right;
            expr = binop;
        }
        else {
            break;
        }
    }
    
    return expr;
}

ASTNode* parse_term() {
    ASTNode* expr = parse_factor();
    
    while (current_token) {
        if (match(TOK_PLUS)) {
            advance();
            ASTNode* right = parse_factor();
            ASTNode* binop = malloc(sizeof(ASTNode));
            binop->type = AST_BINOP;
            binop->line = current_token->line;
            binop->data.binop.left = expr;
            binop->data.binop.op = strdup("+");
            binop->data.binop.right = right;
            expr = binop;
        }
        else if (match(TOK_MINUS)) {
            advance();
            ASTNode* right = parse_factor();
            ASTNode* binop = malloc(sizeof(ASTNode));
            binop->type = AST_BINOP;
            binop->line = current_token->line;
            binop->data.binop.left = expr;
            binop->data.binop.op = strdup("-");
            binop->data.binop.right = right;
            expr = binop;
        }
        else {
            break;
        }
    }
    
    return expr;
}

ASTNode* parse_factor() {
    ASTNode* expr = parse_unary();
    
    while (current_token) {
        if (match(TOK_TIMES)) {
            advance();
            ASTNode* right = parse_unary();
            ASTNode* binop = malloc(sizeof(ASTNode));
            binop->type = AST_BINOP;
            binop->line = current_token->line;
            binop->data.binop.left = expr;
            binop->data.binop.op = strdup("*");
            binop->data.binop.right = right;
            expr = binop;
        }
        else if (match(TOK_DIVIDE)) {
            advance();
            ASTNode* right = parse_unary();
            ASTNode* binop = malloc(sizeof(ASTNode));
            binop->type = AST_BINOP;
            binop->line = current_token->line;
            binop->data.binop.left = expr;
            binop->data.binop.op = strdup("/");
            binop->data.binop.right = right;
            expr = binop;
        }
        else {
            break;
        }
    }
    
    return expr;
}

ASTNode* parse_unary() {
    if (match(TOK_MINUS)) {
        advance();
        ASTNode* operand = parse_unary();
        ASTNode* unary = malloc(sizeof(ASTNode));
        unary->type = AST_UNARYOP;
        unary->line = current_token->line;
        unary->data.unaryop.op = strdup("-");
        unary->data.unaryop.operand = operand;
        return unary;
    }
    
    return parse_primary();
}

ASTNode* parse_primary() {
    if (match(TOK_NUMBER)) {
        char* value = strdup(current_token->value);
        int line = current_token->line;
        advance();
        ASTNode* num = malloc(sizeof(ASTNode));
        num->type = AST_NUMBER;
        num->line = line;
        num->data.number = atof(value);
        free(value);
        return num;
    }
    else if (match(TOK_STRING)) {
        char* value = strdup(current_token->value);
        int line = current_token->line;
        advance();
        ASTNode* str = malloc(sizeof(ASTNode));
        str->type = AST_STRING;
        str->line = line;
        str->data.string = value;
        return str;
    }
    else if (match(KW_TRUE)) {
        advance();
        ASTNode* bool_node = malloc(sizeof(ASTNode));
        bool_node->type = AST_BOOLEAN;
        bool_node->line = current_token->line;
        bool_node->data.boolean = true;
        return bool_node;
    }
    else if (match(KW_FALSE)) {
        advance();
        ASTNode* bool_node = malloc(sizeof(ASTNode));
        bool_node->type = AST_BOOLEAN;
        bool_node->line = current_token->line;
        bool_node->data.boolean = false;
        return bool_node;
    }
    else if (match(KW_NULL)) {
        advance();
        ASTNode* null_node = malloc(sizeof(ASTNode));
        null_node->type = AST_NULL;
        null_node->line = current_token->line;
        return null_node;
    }
    else if (match(TOK_ID)) {
        char* id_name = strdup(current_token->value);
        int line = current_token->line;
        advance();

        // Check if it's a function call
        if (match(TOK_LPAREN)) {
            advance(); // consume '('
            ASTNode** args = malloc(sizeof(ASTNode*) * 10);
            int arg_count = 0;
            if (!match(TOK_RPAREN)) {
                do {
                    args[arg_count++] = parse_expression();
                } while (match(TOK_COMMA) && (advance(), true));
            }
            consume(TOK_RPAREN, "Expected ')'");

            ASTNode* call = malloc(sizeof(ASTNode));
            call->type = AST_CALL;
            call->line = line;
            call->data.call.name = id_name;
            call->data.call.args = args;
            call->data.call.arg_count = arg_count;
            return call;
        }

        // Otherwise, it's a variable
        ASTNode* var = malloc(sizeof(ASTNode));
        var->type = AST_VAR;
        var->line = line;
        var->data.var_name = id_name;
        return var;
    }
    else if (match(TOK_LBRACKET)) {
        advance();
        ASTNode** items = malloc(sizeof(ASTNode*) * 10);
        int count = 0;
        
        if (!match(TOK_RBRACKET)) {
            do {
                items[count++] = parse_expression();
            } while (match(TOK_COMMA) && (advance(), true));
            consume(TOK_RBRACKET, "Expected ']'");
        }
        
        ASTNode* list = malloc(sizeof(ASTNode));
        list->type = AST_LIST;
        list->line = current_token->line;
        list->data.list.items = items;
        list->data.list.count = count;
        return list;
    }
    else if (match(TOK_LPAREN)) {
        advance();
        ASTNode* expr = parse_expression();
        consume(TOK_RPAREN, "Expected ')'");
        return expr;
    }
    
    fprintf(stderr, "Parse error at line %d: Unexpected token\n", 
            current_token ? current_token->line : -1);
    exit(1);
}

// Function to print the AST (for debugging)
void print_ast(ASTNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) printf("  ");

    switch (node->type) {
        case AST_PROGRAM:
            printf("PROGRAM (statements: %d)\n", node->data.list.count);
            for (int i = 0; i < node->data.list.count; i++) {
                print_ast(node->data.list.items[i], indent + 1);
            }
            break;
        case AST_ASSIGN:
            printf("ASSIGN: %s\n", node->data.assign.name);
            print_ast(node->data.assign.value, indent + 1);
            break;
        case AST_BINOP:
            printf("BINOP: %s\n", node->data.binop.op);
            print_ast(node->data.binop.left, indent + 1);
            print_ast(node->data.binop.right, indent + 1);
            break;
        case AST_UNARYOP:
            printf("UNARYOP: %s\n", node->data.unaryop.op);
            print_ast(node->data.unaryop.operand, indent + 1);
            break;
        case AST_NUMBER:
            printf("NUMBER: %g\n", node->data.number);
            break;
        case AST_STRING:
            printf("STRING: %s\n", node->data.string);
            break;
        case AST_BOOLEAN:
            printf("BOOLEAN: %s\n", node->data.boolean ? "true" : "false");
            break;
        case AST_NULL:
            printf("NULL\n");
            break;
        case AST_VAR:
            printf("VAR: %s\n", node->data.var_name);
            break;
        case AST_LIST:
            printf("LIST (items: %d)\n", node->data.list.count);
            for (int i = 0; i < node->data.list.count; i++) {
                print_ast(node->data.list.items[i], indent + 1);
            }
            break;
        case AST_CALL:
            printf("CALL: %s (args: %d)\n", node->data.call.name, node->data.call.arg_count);
            for (int i = 0; i < node->data.call.arg_count; i++) {
                print_ast(node->data.call.args[i], indent + 1);
            }
            break;
        case AST_IF:
    printf("IF\n");
    print_ast(node->data.if_stmt.condition, indent + 1);
    printf("%*sTHEN:\n", indent * 2, "");
    for (int i = 0; i < node->data.if_stmt.then_body_count; i++) {
        print_ast(node->data.if_stmt.then_body[i], indent + 1);
    }
    if (node->data.if_stmt.else_body) {
        printf("%*sELSE:\n", indent * 2, "");
        for (int i = 0; i < node->data.if_stmt.else_body_count; i++) {
            print_ast(node->data.if_stmt.else_body[i], indent + 1);
        }
    }
    break;
        case AST_LOOP:
            printf("LOOP\n");
            print_ast(node->data.loop.condition, indent + 1);
            for (int i = 0; i < node->data.loop.body_count; i++) {
                print_ast(node->data.loop.body[i], indent + 1);
            }
            break;
        case AST_EACH:
            printf("EACH: %s\n", node->data.each.target);
            print_ast(node->data.each.iterable, indent + 1);
            for (int i = 0; i < node->data.each.body_count; i++) {
                print_ast(node->data.each.body[i], indent + 1);
            }
            break;
        case AST_EXPRSTMT:
            printf("EXPRSTMT\n");
            print_ast(node->data.expr, indent + 1);
            break;
        default:
            printf("UNKNOWN AST NODE TYPE: %d\n", node->type);
    }
}
