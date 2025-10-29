# mas_lexer.py — FULL CORRECTED VERSION

import re
import ply.lex as lex

reserved = {
    'loop': 'LOOP',
    'each': 'EACH',
    'in': 'IN',
    'stop': 'STOP',
    'next': 'NEXT',
    'give': 'GIVE',
    'if': 'IF',
    'elif': 'ELIF',
    'else': 'ELSE',
    'def': 'DEF',
    'true': 'TRUE',      # ← ADD
    'false': 'FALSE',    # ← ADD
    'null': 'NULL',      # ← ADD
    'end' : 'END',
}

tokens = [
    'ID',
    'NUMBER',
    'STRING',
    'PLUS', 'MINUS', 'TIMES', 'DIVIDE',
    'EQ', 'NEQ', 'LT', 'LE', 'GT', 'GE',
    'ASSIGN',
    'LPAREN', 'RPAREN',
    'LBRACKET', 'RBRACKET',
    'LBRACE', 'RBRACE',
    'COMMA', 'COLON',
    'NEWLINE',
    # 'INDENT',
    # 'DEDENT',
] + list(reserved.values())

# At the bottom of mas_lexer.py, after `tokens = [...]`
print("Lexer tokens:", tokens)

t_ignore = ' \t'

t_PLUS    = r'\+'
t_MINUS   = r'-'
t_TIMES   = r'\*'
t_DIVIDE  = r'/'
t_EQ      = r'=='
t_NEQ     = r'!='
t_LT      = r'<'
t_LE      = r'<='
t_GT      = r'>'
t_GE      = r'>='
t_ASSIGN  = r'='
t_LPAREN  = r'\('
t_RPAREN  = r'\)'
t_LBRACKET = r'\['
t_RBRACKET = r'\]'
t_LBRACE  = r'\{'
t_RBRACE  = r'\}'
t_COMMA   = r','
t_COLON   = r':'

def t_NUMBER(t):
    r'\d+(\.\d+)?'
    t.value = float(t.value) if '.' in t.value else int(t.value)
    return t

# In mas_lexer.py, replace t_STRING with:

def t_STRING(t):
    r'''("(?:[^"\\]|\\.)*")|('(?:[^'\\]|\\.)*')'''
    # Remove surrounding quotes
    s = t.value
    if s.startswith('"') and s.endswith('"'):
        t.value = s[1:-1].replace(r'\"', '"').replace(r'\\', '\\')
    elif s.startswith("'") and s.endswith("'"):
        t.value = s[1:-1].replace(r"\'", "'").replace(r'\\', '\\')
    return t

def t_ID(t):
    r'[a-zA-Z_][a-zA-Z_0-9]*'
    t.type = reserved.get(t.value, 'ID')
    return t

def t_NEWLINE(t):
    r'\n+'
    t.lexer.lineno += len(t.value)
    # Only return NEWLINE if next line is not blank
    # But simpler: always return NEWLINE, and let parser ignore it via error recovery
    return t

def t_error(t):
    print(f"Illegal character '{t.value[0]}' at line {t.lineno}")
    t.lexer.skip(1)
    
def t_ignore_COMMENT(t):
    r'\#.*'
    pass

def process_indents(code):
    """Preprocess code to insert INDENT/DEDENT tokens (simplified)."""
    lines = code.split('\n')
    indents = [0]
    result = []
    for line in lines:
        if not line.strip():
            continue
        # Count leading spaces (only spaces, no tabs)
        indent = len(line) - len(line.lstrip(' '))
        if indent % 4 != 0:
            raise IndentationError(f"Invalid indentation at line: {line}")
        indent //= 4
        if indent > indents[-1]:
            result.append('INDENT')
            indents.append(indent)
        while indent < indents[-1]:
            result.append('DEDENT')
            indents.pop()
        result.append(line.strip())
    while len(indents) > 1:
        result.append('DEDENT')
        indents.pop()
    return '\n'.join(result)

lexer = lex.lex()