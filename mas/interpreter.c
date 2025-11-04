// interpreter.c
#include "mas.h"

typedef struct {
    char** names;
    MASObject** values;
    int count;
    int capacity;
} SymbolTable;

typedef struct {
    SymbolTable* globals;
    SymbolTable* locals;
} Interpreter;

static MASObject* evaluate(ASTNode* node, Interpreter* interp);
static MASObject* create_number(double value);
static MASObject* create_string(const char* value);
static MASObject* create_boolean(bool value);
static MASObject* create_null();
static MASObject* create_list(MASObject** items, int count);

// Reference counting
void mas_object_incref(MASObject* obj) {
    if (obj) obj->refcount++;
}

void mas_object_decref(MASObject* obj) {
    if (!obj) return;
    obj->refcount--;
    if (obj->refcount <= 0) {
        // Free based on type
        switch (obj->type) {
            case AST_STRING:
                free(obj->data.string);
                break;
            case AST_LIST:
                for (int i = 0; i < obj->data.list.count; i++) {
                    mas_object_decref(obj->data.list.items[i]);
                }
                free(obj->data.list.items);
                break;
            default:
                break;
        }
        free(obj);
    }
}

// Symbol table operations
SymbolTable* create_symbol_table() {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    table->capacity = 16;
    table->count = 0;
    table->names = malloc(sizeof(char*) * table->capacity);
    table->values = malloc(sizeof(MASObject*) * table->capacity);
    return table;
}

void symbol_table_set(SymbolTable* table, const char* name, MASObject* value) {
    // Check if name exists
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->names[i], name) == 0) {
            mas_object_decref(table->values[i]);
            table->values[i] = value;
            mas_object_incref(value);
            return;
        }
    }
    
    // Add new entry
    if (table->count >= table->capacity) {
        table->capacity *= 2;
        table->names = realloc(table->names, sizeof(char*) * table->capacity);
        table->values = realloc(table->values, sizeof(MASObject*) * table->capacity);
    }
    
    table->names[table->count] = strdup(name);
    table->values[table->count] = value;
    mas_object_incref(value);
    table->count++;
}

MASObject* symbol_table_get(SymbolTable* table, const char* name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->names[i], name) == 0) {
            return table->values[i];
        }
    }
    return NULL;
}

// Object creation
static MASObject* create_number(double value) {
    MASObject* obj = malloc(sizeof(MASObject));
    obj->refcount = 1;
    obj->type = AST_NUMBER;
    obj->data.number = value;
    return obj;
}

static MASObject* create_string(const char* value) {
    MASObject* obj = malloc(sizeof(MASObject));
    obj->refcount = 1;
    obj->type = AST_STRING;
    obj->data.string = strdup(value);
    return obj;
}

static MASObject* create_boolean(bool value) {
    MASObject* obj = malloc(sizeof(MASObject));
    obj->refcount = 1;
    obj->type = AST_BOOLEAN;
    obj->data.boolean = value;
    return obj;
}

static MASObject* create_null() {
    MASObject* obj = malloc(sizeof(MASObject));
    obj->refcount = 1;
    obj->type = AST_NULL;
    return obj;
}

static MASObject* create_list(MASObject** items, int count) {
    MASObject* obj = malloc(sizeof(MASObject));
    obj->refcount = 1;
    obj->type = AST_LIST;
    obj->data.list.count = count;
    obj->data.list.items = malloc(sizeof(MASObject*) * count);
    for (int i = 0; i < count; i++) {
        obj->data.list.items[i] = items[i];
        mas_object_incref(items[i]);
    }
    return obj;
}

// Built-in functions
static MASObject* builtin_print(Interpreter* interp, MASObject** args, int arg_count) {
     (void)interp;
    for (int i = 0; i < arg_count; i++) {
        if (i > 0) printf(" ");
        MASObject* arg = args[i];
        switch (arg->type) {
            case AST_NUMBER:
                printf("%g", arg->data.number);
                break;
            case AST_STRING:
                printf("%s", arg->data.string);
                break;
            case AST_BOOLEAN:
                printf("%s", arg->data.boolean ? "true" : "false");
                break;
            case AST_NULL:
                printf("null");
                break;
            case AST_LIST:
                printf("[");
                for (int j = 0; j < arg->data.list.count; j++) {
                    if (j > 0) printf(", ");
                    // Recursively print list items (simplified)
                    MASObject* item = arg->data.list.items[j];
                    if (item->type == AST_NUMBER) {
                        printf("%g", item->data.number);
                    } else if (item->type == AST_STRING) {
                        printf("%s", item->data.string);
                    } else {
                        printf("<object>");
                    }
                }
                printf("]");
                break;
            default:
                printf("<object>");
                break;
        }
    }
    printf("\n");
    return create_null();
}

// Evaluation functions
static MASObject* evaluate_binop(ASTNode* node, Interpreter* interp) {
    MASObject* left = evaluate(node->data.binop.left, interp);
    MASObject* right = evaluate(node->data.binop.right, interp);
    
    // Only support number operations for now
    if (left->type != AST_NUMBER || right->type != AST_NUMBER) {
        fprintf(stderr, "Type error: binary operation requires numbers\n");
        exit(1);
    }
    
    double result;
    if (strcmp(node->data.binop.op, "+") == 0) {
        result = left->data.number + right->data.number;
    } else if (strcmp(node->data.binop.op, "-") == 0) {
        result = left->data.number - right->data.number;
    } else if (strcmp(node->data.binop.op, "*") == 0) {
        result = left->data.number * right->data.number;
    } else if (strcmp(node->data.binop.op, "/") == 0) {
        if (right->data.number == 0) {
            fprintf(stderr, "Division by zero\n");
            exit(1);
        }
        result = left->data.number / right->data.number;
    } else if (strcmp(node->data.binop.op, "==") == 0) {
        mas_object_decref(left);
        mas_object_decref(right);
        return create_boolean(left->data.number == right->data.number);
    } else if (strcmp(node->data.binop.op, "!=") == 0) {
        mas_object_decref(left);
        mas_object_decref(right);
        return create_boolean(left->data.number != right->data.number);
    } else if (strcmp(node->data.binop.op, "<") == 0) {
        mas_object_decref(left);
        mas_object_decref(right);
        return create_boolean(left->data.number < right->data.number);
    } else if (strcmp(node->data.binop.op, "<=") == 0) {
        mas_object_decref(left);
        mas_object_decref(right);
        return create_boolean(left->data.number <= right->data.number);
    } else if (strcmp(node->data.binop.op, ">") == 0) {
        mas_object_decref(left);
        mas_object_decref(right);
        return create_boolean(left->data.number > right->data.number);
    } else if (strcmp(node->data.binop.op, ">=") == 0) {
        mas_object_decref(left);
        mas_object_decref(right);
        return create_boolean(left->data.number >= right->data.number);
    } else {
        fprintf(stderr, "Unknown operator: %s\n", node->data.binop.op);
        exit(1);
    }
    
    mas_object_decref(left);
    mas_object_decref(right);
    return create_number(result);
}

static MASObject* evaluate(ASTNode* node, Interpreter* interp) {
    switch (node->type) {
        case AST_PROGRAM: {
            MASObject* last = create_null();
            for (int i = 0; i < node->data.list.count; i++) {
                mas_object_decref(last);
                last = evaluate(node->data.list.items[i], interp);
            }
            return last;
        }
        case AST_NUMBER:
            return create_number(node->data.number);
        case AST_STRING:
            return create_string(node->data.string);
        case AST_BOOLEAN:
            return create_boolean(node->data.boolean);
        case AST_NULL:
            return create_null();
        case AST_VAR: {
            MASObject* value = symbol_table_get(interp->locals, node->data.var_name);
            if (!value) {
                value = symbol_table_get(interp->globals, node->data.var_name);
            }
            if (!value) {
                fprintf(stderr, "Undefined variable: %s\n", node->data.var_name);
                exit(1);
            }
            mas_object_incref(value);
            return value;
        }
        case AST_ASSIGN: {
            MASObject* value = evaluate(node->data.assign.value, interp);
            symbol_table_set(interp->locals, node->data.assign.name, value);
            return value;
        }
        case AST_BINOP:
            return evaluate_binop(node, interp);
        case AST_UNARYOP: {
            MASObject* operand = evaluate(node->data.unaryop.operand, interp);
            if (operand->type != AST_NUMBER) {
                fprintf(stderr, "Unary minus requires a number\n");
                exit(1);
            }
            MASObject* result = create_number(-operand->data.number);
            mas_object_decref(operand);
            return result;
        }
        case AST_LIST: {
            MASObject** items = malloc(sizeof(MASObject*) * node->data.list.count);
            for (int i = 0; i < node->data.list.count; i++) {
                items[i] = evaluate(node->data.list.items[i], interp);
            }
            MASObject* list = create_list(items, node->data.list.count);
            free(items);
            return list;
        }
        case AST_CALL: {
            // Check for built-in functions
            if (strcmp(node->data.call.name, "print") == 0) {
                MASObject** args = malloc(sizeof(MASObject*) * node->data.call.arg_count);
                for (int i = 0; i < node->data.call.arg_count; i++) {
                    args[i] = evaluate(node->data.call.args[i], interp);
                }
                MASObject* result = builtin_print(interp, args, node->data.call.arg_count);
                for (int i = 0; i < node->data.call.arg_count; i++) {
                    mas_object_decref(args[i]);
                }
                free(args);
                return result;
            }
            
            // TODO: User-defined functions
            fprintf(stderr, "Function not implemented: %s\n", node->data.call.name);
            exit(1);
        }
        case AST_LOOP: {
            while (1) {
                MASObject* cond = evaluate(node->data.loop.condition, interp);
                if (cond->type != AST_BOOLEAN) {
                    fprintf(stderr, "Loop condition must be boolean\n");
                    exit(1);
                }
                if (!cond->data.boolean) {
                    mas_object_decref(cond);
                    break;
                }
                mas_object_decref(cond);
                
                // Execute loop body
                for (int i = 0; i < node->data.loop.body_count; i++) {
                    MASObject* result = evaluate(node->data.loop.body[i], interp);
                    mas_object_decref(result);
                }
            }
            return create_null();
        }
        case AST_EACH: {
            MASObject* iterable = evaluate(node->data.each.iterable, interp);
            if (iterable->type != AST_LIST) {
                fprintf(stderr, "Each requires a list\n");
                exit(1);
            }
            
            for (int i = 0; i < iterable->data.list.count; i++) {
                // Bind target variable
                symbol_table_set(interp->locals, node->data.each.target, iterable->data.list.items[i]);
                
                // Execute body
                for (int j = 0; j < node->data.each.body_count; j++) {
                    MASObject* result = evaluate(node->data.each.body[j], interp);
                    mas_object_decref(result);
                }
            }
            
            mas_object_decref(iterable);
            return create_null();
        }
        case AST_IF: {
            MASObject* cond = evaluate(node->data.loop.condition, interp);
            if (cond->type != AST_BOOLEAN) {
                fprintf(stderr, "If condition must be boolean\n");
                exit(1);
            }
            if (cond->data.boolean) {
                mas_object_decref(cond);
                // Execute if body
                for (int i = 0; i < node->data.loop.body_count; i++) {
                    MASObject* result = evaluate(node->data.loop.body[i], interp);
                    mas_object_decref(result);
                }
            } else {
                mas_object_decref(cond);
            }
            return create_null();
        }
        case AST_EXPRSTMT: {
            MASObject* result = evaluate(node->data.expr, interp);
            mas_object_decref(result);
            return create_null();
        }
        case AST_BREAK:
        case AST_CONTINUE:
        case AST_RETURN:
        case AST_FUNCDEF:
            // TODO: Implement these
            return create_null();
        default:
            fprintf(stderr, "Unknown AST node type: %d\n", node->type);
            exit(1);
    }
}

MASObject* interpret(ASTNode* ast) {
    Interpreter interp;
    interp.globals = create_symbol_table();
    interp.locals = create_symbol_table();
    
    MASObject* result = evaluate(ast, &interp);
    
    // Cleanup
    // TODO: Proper cleanup of symbol tables
    
    return result;
}