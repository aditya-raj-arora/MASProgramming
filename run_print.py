# run_print.py
from mas_lexer import lexer
from mas_parser import parser
from mas_interpreter import MASInterpreter

code = """def add(a, b):
    give a + b
end
x = 10
loop x > 0:
    x = x - 1
end
each item in [1, 2]:
    if item == 1:
        print("Found one!")
    end
end
result = add(3, 4)
print("Result:", result)
"""
if not code.endswith('\n'):
    code += '\n'
print("Tokens:")
lexer.input(code)
for tok in lexer:
    print(tok)

print(" Parsing...")
ast = parser.parse(code, lexer=lexer)


from mas_ast import dump_ast
if ast:
    print("\n AST:")
    dump_ast(ast)

if ast:
    print(" Parsed successfully!")
    print("\n Interpreting...\n")
    interp = MASInterpreter()
    interp.visit(ast)
    print("\n Done!")
else:
    print(" Parse failed")