// parser.c
#include "mas.h"

static Token* current_token = NULL;
static Token* next_token = NULL;

static void advance() {
    if (next_token) free(next_token);
    next_token = lexer_next();
    current_token = next_token;
}

static bool match(TokenType type) {
    if (current_token && current_token->type == type) {
        advance();
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
}

// Forward declarations for recursive parsing
ASTNode* parse_statement();
ASTNode* parse_expression();
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
    }
    
    program->data.list.items = statements;
    program->data.list.count = stmt_count;
    return program;
}

// Parse statement
ASTNode* parse_statement() {
    if (match(KW_DEF)) {
        // Function definition
        consume(TOK_ID, "Expected function name");
        char* func_name = strdup(current_token->value);
        consume(TOK_LPAREN, "Expected '('");
        
        char** params = malloc(sizeof(char*) * 10);
        int param_count = 0;
        
        if (!match(TOK_RPAREN)) {
            do {
                consume(TOK_ID, "Expected parameter name");
                params[param_count++] = strdup(current_token->value);
            } while (match(TOK_COMMA));
            consume(TOK_RPAREN, "Expected ')'");
        }
        
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
        func->line = current_token->line;
        func->data.funcdef.name = func_name;
        func->data.funcdef.params = params;
        func->data.funcdef.param_count = param_count;
        func->data.funcdef.body = body;
        func->data.funcdef.body_count = body_count;
        return func;
    }
    else if (match(KW_LOOP)) {
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
        loop->line = current_token->line;
        loop->data.loop.condition = condition;
        loop->data.loop.body = body;
        loop->data.loop.body_count = body_count;
        return loop;
    }
    else if (match(KW_EACH)) {
        consume(TOK_ID, "Expected variable name");
        char* target = strdup(current_token->value);
        consume(KW_IN, "Expected 'in'");
        ASTNode* iterable = parse_expression();
        consume(TOK_COLON, "Expected ':'");
        consume(TOK_NEWLINE, "Expected newline after each header");
        
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
        each->line = current_token->line;
        each->data.each.target = target;
        each->data.each.iterable = iterable;
        each->data.each.body = body;
        each->data.each.body_count = body_count;
        return each;
    }
    else if (match(KW_IF)) {
        ASTNode* condition = parse_expression();
        consume(TOK_COLON, "Expected ':'");
        consume(TOK_NEWLINE, "Expected newline after if condition");
        
        int body_count = 0;
        ASTNode** body = malloc(sizeof(ASTNode*) * 100);
        while (current_token && current_token->type != TOK_END) {
            if (current_token->type == TOK_NEWLINE) {
                advance();
                continue;
            }
            body[body_count++] = parse_statement();
        }
        consume(TOK_END, "Expected 'end' to close if");
        
        ASTNode* if_stmt = malloc(sizeof(ASTNode));
        if_stmt->type = AST_IF;
        if_stmt->line = current_token->line;
        if_stmt->data.loop.condition = condition;
        if_stmt->data.loop.body = body;
        if_stmt->data.loop.body_count = body_count;
        return if_stmt;
    }
    else if (match(KW_GIVE)) {
        ASTNode* value = parse_expression();
        ASTNode* ret = malloc(sizeof(ASTNode));
        ret->type = AST_RETURN;
        ret->line = current_token->line;
        ret->data.expr = value;
        return ret;
    }
    else if (match(KW_STOP)) {
        ASTNode* brk = malloc(sizeof(ASTNode));
        brk->type = AST_BREAK;
        brk->line = current_token->line;
        return brk;
    }
    else if (match(KW_NEXT)) {
        ASTNode* cont = malloc(sizeof(ASTNode));
        cont->type = AST_CONTINUE;
        cont->line = current_token->line;
        return cont;
    }
    else if (current_token->type == TOK_ID) {
        char* id = strdup(current_token->value);
        advance();
        if (match(TOK_ASSIGN)) {
            // Assignment
            ASTNode* value = parse_expression();
            ASTNode* assign = malloc(sizeof(ASTNode));
            assign->type = AST_ASSIGN;
            assign->line = current_token->line;
            assign->data.assign.name = id;
            assign->data.assign.value = value;
            return assign;
        }
        else if (match(TOK_LPAREN)) {
            // Function call
            ASTNode** args = malloc(sizeof(ASTNode*) * 10);
            int arg_count = 0;
            
            if (!match(TOK_RPAREN)) {
                do {
                    args[arg_count++] = parse_expression();
                } while (match(TOK_COMMA));
                consume(TOK_RPAREN, "Expected ')'");
            }
            
            ASTNode* call = malloc(sizeof(ASTNode));
            call->type = AST_CALL;
            call->line = current_token->line;
            call->data.call.name = id;
            call->data.call.args = args;
            call->data.call.arg_count = arg_count;
            return call;
        }
        else {
            // Standalone expression (variable)
            ASTNode* var = malloc(sizeof(ASTNode));
            var->type = AST_VAR;
            var->line = current_token->line;
            var->data.var_name = id;
            return var;
        }
    }
    
    // Default: parse as expression statement
    ASTNode* expr = parse_expression();
    ASTNode* stmt = malloc(sizeof(ASTNode));
    stmt->type = AST_EXPRSTMT;
    stmt->line = current_token->line;
    stmt->data.expr = expr;
    return stmt;
}

// Parse expression (simplified - left associative)
ASTNode* parse_expression() {
    return parse_comparison();
}

ASTNode* parse_comparison() {
    ASTNode* expr = parse_term();
    
    while (current_token) {
        if (match(TOK_EQ)) {
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
        ASTNode* num = malloc(sizeof(ASTNode));
        num->type = AST_NUMBER;
        num->line = current_token->line;
        num->data.number = atof(current_token->value);
        return num;
    }
    else if (match(TOK_STRING)) {
        ASTNode* str = malloc(sizeof(ASTNode));
        str->type = AST_STRING;
        str->line = current_token->line;
        str->data.string = strdup(current_token->value);
        return str;
    }
    else if (match(KW_TRUE)) {
        ASTNode* bool_node = malloc(sizeof(ASTNode));
        bool_node->type = AST_BOOLEAN;
        bool_node->line = current_token->line;
        bool_node->data.boolean = true;
        return bool_node;
    }
    else if (match(KW_FALSE)) {
        ASTNode* bool_node = malloc(sizeof(ASTNode));
        bool_node->type = AST_BOOLEAN;
        bool_node->line = current_token->line;
        bool_node->data.boolean = false;
        return bool_node;
    }
    else if (match(KW_NULL)) {
        ASTNode* null_node = malloc(sizeof(ASTNode));
        null_node->type = AST_NULL;
        null_node->line = current_token->line;
        return null_node;
    }
    else if (match(TOK_ID)) {
        ASTNode* var = malloc(sizeof(ASTNode));
        var->type = AST_VAR;
        var->line = current_token->line;
        var->data.var_name = strdup(current_token->value);
        return var;
    }
    else if (match(TOK_LBRACKET)) {
        ASTNode** items = malloc(sizeof(ASTNode*) * 10);
        int count = 0;
        
        if (!match(TOK_RBRACKET)) {
            do {
                items[count++] = parse_expression();
            } while (match(TOK_COMMA));
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
        ASTNode* expr = parse_expression();
        consume(TOK_RPAREN, "Expected ')'");
        return expr;
    }
    
    fprintf(stderr, "Parse error at line %d: Unexpected token\n", 
            current_token ? current_token->line : -1);
    exit(1);
}