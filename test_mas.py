# test_mas.py
from mas_lexer import lexer
from mas_parser import parser
from mas_interpreter import MASInterpreter

# Note: Every simple statement ends with \n
code = """x = 10
loop x > 0:
    x = x - 1
end
print("Loop done!")
each item in [1, 2, 3]:
    if item == 2:
        stop
    print("Item:", item)
end"""

# Ensure the entire string ends with a newline
if not code.endswith('\n'):
    code += '\n'

print("🔍 Parsing...")
ast = parser.parse(code, lexer=lexer)

if ast is None:
    print("❌ Parsing failed.")
else:
    print("✅ Parsing succeeded!\n")
    print("🚀 Interpreting...\n")
    interp = MASInterpreter()
    interp.visit(ast)