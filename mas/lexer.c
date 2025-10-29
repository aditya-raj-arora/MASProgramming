// lexer.c
#include "mas.h"

static char* current;
static int line = 1;
static FILE* file;

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
    if (current && *current) {
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
        } else if (c == '#') {
            // Comment
            while ((c = next_char()) != EOF && c != '\n');
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
    if (c == EOF) {
        Token* tok = malloc(sizeof(Token));
        tok->type = TOK_EOF;
        tok->value = NULL;
        tok->line = line;
        return tok;
    }
    
    // Single character tokens
    if (c == '+') { next_char(); return &(Token){TOK_PLUS, "+", line}; }
    if (c == '-') { next_char(); return &(Token){TOK_MINUS, "-", line}; }
    if (c == '*') { next_char(); return &(Token){TOK_TIMES, "*", line}; }
    if (c == '/') { next_char(); return &(Token){TOK_DIVIDE, "/", line}; }
    if (c == '(') { next_char(); return &(Token){TOK_LPAREN, "(", line}; }
    if (c == ')') { next_char(); return &(Token){TOK_RPAREN, ")", line}; }
    if (c == '[') { next_char(); return &(Token){TOK_LBRACKET, "[", line}; }
    if (c == ']') { next_char(); return &(Token){TOK_RBRACKET, "]", line}; }
    if (c == '{') { next_char(); return &(Token){TOK_LBRACE, "{", line}; }
    if (c == '}') { next_char(); return &(Token){TOK_RBRACE, "}", line}; }
    if (c == ',') { next_char(); return &(Token){TOK_COMMA, ",", line}; }
    if (c == ':') { next_char(); return &(Token){TOK_COLON, ":", line}; }
    if (c == '=') {
        next_char();
        if (peek_char() == '=') {
            next_char();
            return &(Token){TOK_EQ, "==", line};
        }
        return &(Token){TOK_ASSIGN, "=", line};
    }
    if (c == '!') {
        next_char();
        if (peek_char() == '=') {
            next_char();
            return &(Token){TOK_NEQ, "!=", line};
        }
        // Error: unexpected '!'
        Token* tok = malloc(sizeof(Token));
        tok->type = TOK_ERROR;
        tok->value = strdup("Unexpected '!'"); 
        tok->line = line;
        return tok;
    }
    if (c == '<') {
        next_char();
        if (peek_char() == '=') {
            next_char();
            return &(Token){TOK_LE, "<=", line};
        }
        return &(Token){TOK_LT, "<", line};
    }
    if (c == '>') {
        next_char();
        if (peek_char() == '=') {
            next_char();
            return &(Token){TOK_GE, ">=", line};
        }
        return &(Token){TOK_GT, ">", line};
    }
    if (c == '\n') {
        next_char();
        return &(Token){TOK_NEWLINE, "\n", line};
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
    
    // Unknown character
    Token* tok = malloc(sizeof(Token));
    tok->type = TOK_ERROR;
    char msg[100];
    sprintf(msg, "Unknown character: %c", c);
    tok->value = strdup(msg);
    tok->line = line;
    next_char(); // consume it
    return tok;
}