// lexer.c
#include "mas.h"

static char* current;
static int line = 1;
static FILE* file;
static bool eof_reached = false;

void lexer_init(FILE* f) {
    file = f;
    current = NULL;
}

// Helper function to read next character
static int next_char() {    
    if (current && *current) {
        char c = *current++;
        if (c == '\n') line++;
        return c;
    }
    return EOF;
}

// Helper function to peek at next character
static int peek_char() {
    if (current && *current != '\0') {
        return *current;
    }
    return EOF;
}

// Skip whitespace
static void skip_whitespace() {
    int c;
    while ((c = peek_char()) != EOF) {
        if (c == ' ' || c == '\t') {
            next_char();
        } else if (c == '\r') {
            // Skip \r (in case of \r\n or lone \r)
            next_char();
            // If followed by \n, we'll let lexer_next() handle the \n
        } else if (c == '#') {
            next_char(); // consume '#'
            while ((c = peek_char()) != EOF && c != '\n' && c != '\r') {
                next_char();
            }
            // Do NOT consume \n or \r — leave for lexer_next()
        } else {
            break;
        }
    }
}

// Read identifier or keyword
static Token* read_identifier() {
    char buffer[256];
    int i = 0;
    int c = peek_char();
    while ((c != EOF) && (isalpha(c) || c == '_' || (i > 0 && isdigit(c)))) {
        buffer[i++] = next_char();
        c = peek_char();
    }
    buffer[i] = '\0';
    
    Token* tok = malloc(sizeof(Token));
    tok->line = line;
    tok->value = strdup(buffer);
    
    // Check for keywords
    if (strcmp(buffer, "loop") == 0) tok->type = KW_LOOP;
    else if (strcmp(buffer, "each") == 0) tok->type = KW_EACH;
    else if (strcmp(buffer, "in") == 0) tok->type = KW_IN;
    else if (strcmp(buffer, "stop") == 0) tok->type = KW_STOP;
    else if (strcmp(buffer, "next") == 0) tok->type = KW_NEXT;
    else if (strcmp(buffer, "give") == 0) tok->type = KW_GIVE;
    else if (strcmp(buffer, "if") == 0) tok->type = KW_IF;
    else if (strcmp(buffer, "elif") == 0) tok->type = KW_ELIF;
    else if (strcmp(buffer, "else") == 0) tok->type = KW_ELSE;
    else if (strcmp(buffer, "def") == 0) tok->type = KW_DEF;
    else if (strcmp(buffer, "true") == 0) tok->type = KW_TRUE;
    else if (strcmp(buffer, "false") == 0) tok->type = KW_FALSE;
    else if (strcmp(buffer, "null") == 0) tok->type = KW_NULL;
    else if (strcmp(buffer, "print") == 0) tok->type = KW_PRINT;
    else if (strcmp(buffer, "end") == 0) tok->type = TOK_END;
    else tok->type = TOK_ID;
    
    return tok;
}

// Read number
static Token* read_number() {
    char buffer[256];
    int i = 0;
    int c = peek_char();
    bool has_decimal = false;
    
    while ((c != EOF) && (isdigit(c) || c == '.')) {
        if (c == '.') {
            if (has_decimal) break;
            has_decimal = true;
        }
        buffer[i++] = next_char();
        c = peek_char();
    }
    buffer[i] = '\0';
    
    Token* tok = malloc(sizeof(Token));
    tok->type = TOK_NUMBER;
    tok->value = strdup(buffer);
    tok->line = line;
    return tok;
}

// Read string
static Token* read_string() {
    char buffer[1024];
    int i = 0;
    char quote = next_char(); // consume opening quote
    int c = next_char();
    
    while (c != EOF && c != quote) {
        if (c == '\\') {
            c = next_char();
            if (c == 'n') buffer[i++] = '\n';
            else if (c == 't') buffer[i++] = '\t';
            else if (c == '\\' || c == '"' || c == '\'') buffer[i++] = c;
            else {
                buffer[i++] = '\\';
                buffer[i++] = c;
            }
        } else {
            buffer[i++] = c;
        }
        c = next_char();
    }
    buffer[i] = '\0';
    
    if (c != quote) {
        // Unterminated string
        Token* tok = malloc(sizeof(Token));
        tok->type = TOK_ERROR;
        tok->value = strdup("Unterminated string");
        tok->line = line;
        return tok;
    }
    
    Token* tok = malloc(sizeof(Token));
    tok->type = TOK_STRING;
    tok->value = strdup(buffer);
    tok->line = line;
    return tok;
}

static Token* make_token(TokenType type, char* value, int line_num) {
    Token* tok = malloc(sizeof(Token));
    if (!tok) {
        perror("Failed to allocate token");
        exit(1);
    }
    tok->type = type;
    tok->value = value;
    tok->line = line_num;
    return tok;
}

// Main lexer function
Token* lexer_next() {
    if (!current) {
        // Read entire file into memory
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        char* buffer = malloc(size + 1);
        fread(buffer, 1, size, file);
        buffer[size] = '\0';
        current = buffer;
    }
    
    skip_whitespace();
    
    int c = peek_char();
    // Single character tokens
    if (c == '+') { next_char(); return make_token(TOK_PLUS, NULL, line); }
    if (c == '-') { next_char(); return make_token(TOK_MINUS, NULL, line); }
    if (c == '*') { next_char(); return make_token(TOK_TIMES, NULL, line); }
    if (c == '/') { next_char(); return make_token(TOK_DIVIDE, NULL, line); }
    if (c == '(') { next_char(); return make_token(TOK_LPAREN, NULL, line); }
    if (c == ')') { next_char(); return make_token(TOK_RPAREN, NULL, line); }
    if (c == '[') { next_char(); return make_token(TOK_LBRACKET, NULL, line); }
    if (c == ']') { next_char(); return make_token(TOK_RBRACKET, NULL, line); }
    if (c == '{') { next_char(); return make_token(TOK_LBRACE, NULL, line); }
    if (c == '}') { next_char(); return make_token(TOK_RBRACE, NULL, line); }
    if (c == ',') { next_char(); return make_token(TOK_COMMA, NULL, line); }
    if (c == ':') { next_char(); return make_token(TOK_COLON, NULL, line); }
    if (c == '=') {
        next_char();
        if (peek_char() == '=') {
            next_char();
            return make_token(TOK_EQ, NULL, line);
        }
        return make_token(TOK_ASSIGN, NULL, line);
    }
    if (c == '!') {
        next_char();
        if (peek_char() == '=') {
            next_char();
            return make_token(TOK_NEQ, NULL, line);
        }
        // Error: unexpected '!'
        return make_token(TOK_ERROR, strdup("Unexpected '!'"), line);
    }
    if (c == '<') {
        next_char();
        if (peek_char() == '=') {
            next_char();
            return make_token(TOK_LE, NULL, line);
        }
        return make_token(TOK_LT, NULL, line);
    }
    if (c == '>') {
        next_char();
        if (peek_char() == '=') {
            next_char();
            return make_token(TOK_GE, NULL, line);
        }
        return make_token(TOK_GT, NULL, line);
    }
    if (c == '\n') {
        next_char();
        // The newline token belongs to the line we just finished.
        return make_token(TOK_NEWLINE, NULL, line - 1);
    }
    
    // Multi-character tokens
    if (isalpha(c) || c == '_') {
        return read_identifier();
    }
    if (isdigit(c)) {
        return read_number();
    }
    if (c == '"' || c == '\'') {
        return read_string();
    }
    
    if (c == EOF) {
        eof_reached = true;
        return make_token(TOK_EOF, NULL, line);
    }

    // Unknown character
    char msg[100];
    if (isprint(c)) {
        sprintf(msg, "Unknown character: '%c'", c);
    } else {
        sprintf(msg, "Unknown character: '\\x%02X'", (unsigned char)c);
    }
    next_char(); // consume it
    return make_token(TOK_ERROR, strdup(msg), line);
}

// Add this at the bottom of lexer.c (or anywhere after lexer_next is defined)
void print_tokens() {
    Token* tok;
    do {
        tok = lexer_next();
        switch (tok->type) {
            case TOK_EOF:
                printf("EOF\n");
                free(tok);
                break;
            case TOK_ERROR:
                printf("ERROR (line %d): %s\n", tok->line, tok->value);
                break;
            case TOK_NUMBER:
                printf("NUMBER (line %d): %s\n", tok->line, tok->value);
                break;
            case TOK_STRING:
                printf("STRING (line %d): \"%s\"\n", tok->line, tok->value);
                break;
            case TOK_ID:
                printf("IDENTIFIER (line %d): %s\n", tok->line, tok->value);
                break;
            case TOK_NEWLINE:
                printf("NEWLINE (line %d)\n", tok->line);
                break;
            case TOK_PLUS:     printf("PLUS (line %d)\n", tok->line); break;
            case TOK_MINUS:    printf("MINUS (line %d)\n", tok->line); break;
            case TOK_TIMES:    printf("TIMES (line %d)\n", tok->line); break;
            case TOK_DIVIDE:   printf("DIVIDE (line %d)\n", tok->line); break;
            case TOK_ASSIGN:   printf("ASSIGN (line %d)\n", tok->line); break;
            case TOK_EQ:       printf("EQ (line %d)\n", tok->line); break;
            case TOK_NEQ:      printf("NEQ (line %d)\n", tok->line); break;
            case TOK_LT:       printf("LT (line %d)\n", tok->line); break;
            case TOK_LE:       printf("LE (line %d)\n", tok->line); break;
            case TOK_GT:       printf("GT (line %d)\n", tok->line); break;
            case TOK_GE:       printf("GE (line %d)\n", tok->line); break;
            case TOK_LPAREN:   printf("LPAREN (line %d)\n", tok->line); break;
            case TOK_RPAREN:   printf("RPAREN (line %d)\n", tok->line); break;
            case TOK_LBRACKET: printf("LBRACKET (line %d)\n", tok->line); break;
            case TOK_RBRACKET: printf("RBRACKET (line %d)\n", tok->line); break;
            case TOK_LBRACE:   printf("LBRACE (line %d)\n", tok->line); break;
            case TOK_RBRACE:   printf("RBRACE (line %d)\n", tok->line); break;
            case TOK_COMMA:    printf("COMMA (line %d)\n", tok->line); break;
            case TOK_COLON:    printf("COLON (line %d)\n", tok->line); break;
            case TOK_END:      printf("END (line %d)\n", tok->line); break;

            // Keywords
            case KW_LOOP:   printf("KW_LOOP (line %d)\n", tok->line); break;
            case KW_EACH:   printf("KW_EACH (line %d)\n", tok->line); break;
            case KW_IN:     printf("KW_IN (line %d)\n", tok->line); break;
            case KW_STOP:   printf("KW_STOP (line %d)\n", tok->line); break;
            case KW_NEXT:   printf("KW_NEXT (line %d)\n", tok->line); break;
            case KW_GIVE:   printf("KW_GIVE (line %d)\n", tok->line); break;
            case KW_IF:     printf("KW_IF (line %d)\n", tok->line); break;
            case KW_ELIF:   printf("KW_ELIF (line %d)\n", tok->line); break;
            case KW_ELSE:   printf("KW_ELSE (line %d)\n", tok->line); break;
            case KW_DEF:    printf("KW_DEF (line %d)\n", tok->line); break;
            case KW_TRUE:   printf("KW_TRUE (line %d)\n", tok->line); break;
            case KW_FALSE:  printf("KW_FALSE (line %d)\n", tok->line); break;
            case KW_NULL:   printf("KW_NULL (line %d)\n", tok->line); break;
            case KW_PRINT:  printf("KW_PRINT (line %d)\n", tok->line); break;

            default:
                printf("UNKNOWN TOKEN (line %d): %s\n", tok->line, tok->value ? tok->value : "(null)");
                break;
        }

        // Free token memory if you're using malloc in lexer_next
        if (tok->value) free(tok->value);
        free(tok);

    } while (tok->type != TOK_EOF);
}