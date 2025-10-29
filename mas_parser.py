from mas_ast import *
# mas_parser.py - ABSOLUTELY MINIMAL WORKING VERSION
import ply.yacc as yacc
from mas_lexer import tokens, lexer
# Virtual tokens for precedence
tokens = tokens + ['UMINUS']
precedence = (
    ('left', 'EQ', 'NEQ'),
    ('left', 'LT', 'LE', 'GT', 'GE'),
    ('left', 'PLUS', 'MINUS'),
    ('left', 'TIMES', 'DIVIDE'),
    ('right', 'UMINUS'),  # This is fine, but UMINUS isn't a token
)

def p_program(p):
    '''program : statements
               | statements NEWLINE'''
    p[0] = Program(p[1])

   
def p_statements(p):
    '''statements : statement
                  | statements statement
                  | empty'''
    if len(p) == 2:
        # Single statement
        p[0] = p[1] if p[1] is not None else []
    else:
        # statements statement
        stmts1 = p[1] if p[1] is not None else []
        stmts2 = p[2] if p[2] is not None else []
        p[0] = stmts1 + stmts2
        
def p_statement(p):
    '''statement : assign_stmt NEWLINE
                 | call_stmt NEWLINE
                 | GIVE expr NEWLINE
                 | loop_block
                 | each_block
                 | if_block
                 | func_def
                 | STOP NEWLINE
                 | NEXT NEWLINE
                 | NEWLINE'''
    if len(p) == 2:
        # Handles: loop_block, each_block, if_block, func_def, NEWLINE
        if p.slice[1].type == 'NEWLINE':
            p[0] = []  # blank line
        else:
            p[0] = p[1]  # these rules already return lists
    elif len(p) == 3:
        # Handles: assign_stmt, call_stmt, GIVE, STOP, NEXT
        if p[1] == 'stop':
            p[0] = [Break()]
        elif p[1] == 'next':
            p[0] = [Continue()]
        elif p[1] == 'give':
            p[0] = [Return(p[2])]
        else:
            p[0] = [p[1]]  # assign_stmt or call_stmt
    else:
        p[0] = p[1]

def p_call_stmt(p):
    '''call_stmt : ID LPAREN args RPAREN'''
    p[0] = ExprStmt(Call(Var(p[1]), p[3]))
      
def p_assign_stmt(p):
    '''assign_stmt : ID ASSIGN expr'''
    p[0] = Assign(p[1], p[3])
  
def p_loop_block(p):
    '''loop_block : LOOP expr COLON NEWLINE statements END'''
    p[0] = [Loop(p[2], p[5])]

def p_each_block(p):
    '''each_block : EACH ID IN expr COLON NEWLINE statements END'''
    p[0] = [Each(Var(p[2]), p[4], p[7])]

def p_if_block(p):
    '''if_block : IF expr COLON NEWLINE statements END'''
    p[0] = [If(p[2], p[5], [], None)]


def p_func_def(p):
    '''func_def : DEF ID LPAREN params RPAREN COLON NEWLINE statements END'''
    p[0] = [FuncDef(p[2], p[4], p[8])]

def p_params(p):
    '''params : ID
              | params COMMA ID
              | empty'''
    if len(p) == 2:
        p[0] = [p[1]] if p[1] else []
    elif len(p) == 4:
        p[0] = p[1] + [p[3]]
        
def p_args(p):
    '''args : expr_list
            | empty'''
    p[0] = p[1] if p[1] else []

# --- Assignments ---

def p_expr_list(p):
    '''expr_list : expr
                 | expr_list COMMA expr'''
    p[0] = [p[1]] if len(p) == 2 else p[1] + [p[3]]

def p_expr(p):
    '''expr : expr PLUS expr
            | expr MINUS expr
            | expr TIMES expr
            | expr DIVIDE expr
            | expr EQ expr
            | expr NEQ expr
            | expr LT expr
            | expr LE expr
            | expr GT expr
            | expr GE expr
            | MINUS expr %prec UMINUS
            | primary'''
    if len(p) == 4:
        p[0] = BinOp(p[1], p[2], p[3])
    elif len(p) == 3:
        p[0] = UnaryOp(p[1], p[2])
    else:
        p[0] = p[1]

def p_primary(p):
    '''primary : NUMBER
               | STRING
               | TRUE
               | FALSE
               | NULL
               | ID
               | LBRACKET expr_list RBRACKET
               | LBRACKET RBRACKET
               | LPAREN expr RPAREN'''
    if p.slice[1].type == 'NUMBER':
        p[0] = Number(p[1])
    elif p.slice[1].type == 'STRING':
        p[0] = String(p[1])
    elif p.slice[1].type == 'TRUE':
        p[0] = Boolean(True)
    elif p.slice[1].type == 'FALSE':
        p[0] = Boolean(False)
    elif p.slice[1].type == 'NULL':
        p[0] = Null()
    elif p.slice[1].type == 'ID':
        p[0] = Var(p[1])
    elif p.slice[1].type == 'LBRACKET':
        if len(p) == 4:
            p[0] = List(p[2])
        else:
            p[0] = List([])
    else:
        p[0] = p[2]

def p_empty(p):
    '''empty :'''
    pass

def p_error(p):
    if p:
        print(f"Syntax error at '{p.value}' (line {p.lineno})")
    else:
        print("Syntax error at EOF")

parser = yacc.yacc(debug=False, write_tables=False)