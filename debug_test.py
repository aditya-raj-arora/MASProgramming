# debug_test.py
from mas_lexer import lexer
from mas_parser import parser
from mas_interpreter import MASInterpreter

code = "print('hi')\n"  # ← note \n

print("Tokens:")
lexer.input(code)
for tok in lexer:
    print(tok)

print("\nParsing...")
ast = parser.parse(code, lexer=lexer)
print("AST:", "Success" if ast else "Failed")

if ast:
    print("✅ Parsed successfully!")
    print("\n🚀 Interpreting...\n")
    interp = MASInterpreter()
    interp.visit(ast)
    print("\n✨ Done!")
else:
    print("❌ Parse failed")