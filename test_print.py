from mas_lexer import lexer
from mas_parser import parser
from mas_interpreter import MASInterpreter

code = 'print("Hello from MAS!")\n'

ast = parser.parse(code, lexer=lexer)
print("AST:", ast)
if ast:
    interp = MASInterpreter()
    interp.visit(ast)