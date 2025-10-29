# mas_ast.py

class ASTNode:
    pass

class Program(ASTNode):
    def __init__(self, statements):
        self.statements = statements  # list of stmts

class Assign(ASTNode):
    def __init__(self, name, value):
        self.name = name      # str
        self.value = value    # expr

class BinOp(ASTNode):
    def __init__(self, left, op, right):
        self.left = left
        self.op = op          # str: '+', '==', etc.
        self.right = right

class UnaryOp(ASTNode):
    def __init__(self, op, operand):
        self.op = op          # e.g., '-'
        self.operand = operand

class Number(ASTNode):
    def __init__(self, value):
        self.value = value    # int or float

class String(ASTNode):
    def __init__(self, value):
        self.value = value    # str

class Boolean(ASTNode):
    def __init__(self, value):
        self.value = value    # True/False

class Null(ASTNode):
    pass

class Var(ASTNode):
    def __init__(self, name):
        self.name = name

class List(ASTNode):
    def __init__(self, elements):
        self.elements = elements

class Dict(ASTNode):
    def __init__(self, pairs):  # list of (key_expr, value_expr)
        self.pairs = pairs

class Call(ASTNode):
    def __init__(self, func, args):
        self.func = func      # Var or expr
        self.args = args      # list of expr

# Control flow
class If(ASTNode):
    def __init__(self, condition, body, elifs, else_body):
        self.condition = condition   # expr
        self.body = body             # list of stmt
        self.elifs = elifs           # list of (cond, body)
        self.else_body = else_body   # list of stmt or None

class Loop(ASTNode):  # while
    def __init__(self, condition, body):
        self.condition = condition
        self.body = body

class Each(ASTNode):  # for-in
    def __init__(self, target, iterable, body):
        self.target = target      # Var (e.g., 'x')
        self.iterable = iterable  # expr (e.g., [1,2,3])
        self.body = body

class FuncDef(ASTNode):
    def __init__(self, name, params, body):
        self.name = name
        self.params = params  # list of str
        self.body = body      # list of stmt

class Return(ASTNode):  # 'give'
    def __init__(self, value):
        self.value = value  # expr or None

class Break(ASTNode):   # 'stop'
    pass

class Continue(ASTNode):  # 'next'
    pass

class ExprStmt(ASTNode):  # standalone expression (e.g., function call)
    def __init__(self, expr):
        self.expr = expr
        
# mas_ast.py (append at the bottom)

def dump_ast(node, indent=0):
    """Recursively print the AST in a readable format."""
    prefix = "  " * indent
    if isinstance(node, Program):
        print(f"{prefix}Program")
        for stmt in node.statements:
            dump_ast(stmt, indent + 1)
    elif isinstance(node, Assign):
        print(f"{prefix}Assign: {node.name}")
        dump_ast(node.value, indent + 1)
    elif isinstance(node, Var):
        print(f"{prefix}Var: {node.name}")
    elif isinstance(node, Number):
        print(f"{prefix}Number: {node.value}")
    elif isinstance(node, String):
        print(f"{prefix}String: \"{node.value}\"")
    elif isinstance(node, Boolean):
        print(f"{prefix}Boolean: {node.value}")
    elif isinstance(node, Null):
        print(f"{prefix}Null")
    elif isinstance(node, BinOp):
        print(f"{prefix}BinOp: {node.op}")
        dump_ast(node.left, indent + 1)
        dump_ast(node.right, indent + 1)
    elif isinstance(node, UnaryOp):
        print(f"{prefix}UnaryOp: {node.op}")
        dump_ast(node.operand, indent + 1)
    elif isinstance(node, List):
        print(f"{prefix}List")
        for elem in node.elements:
            dump_ast(elem, indent + 1)
    elif isinstance(node, Dict):
        print(f"{prefix}Dict")
        for key, val in node.pairs:
            print(f"{'  ' * (indent + 1)}Key:")
            dump_ast(key, indent + 2)
            print(f"{'  ' * (indent + 1)}Value:")
            dump_ast(val, indent + 2)
    elif isinstance(node, Call):
        print(f"{prefix}Call: {node.func.name if isinstance(node.func, Var) else '...'}")
        for arg in node.args:
            dump_ast(arg, indent + 1)
    elif isinstance(node, Loop):
        print(f"{prefix}Loop")
        print(f"{'  ' * (indent + 1)}Condition:")
        dump_ast(node.condition, indent + 2)
        print(f"{'  ' * (indent + 1)}Body:")
        for stmt in node.body:
            dump_ast(stmt, indent + 2)
    elif isinstance(node, Each):
        print(f"{prefix}Each")
        print(f"{'  ' * (indent + 1)}Target:")
        dump_ast(node.target, indent + 2)
        print(f"{'  ' * (indent + 1)}Iterable:")
        dump_ast(node.iterable, indent + 2)
        print(f"{'  ' * (indent + 1)}Body:")
        for stmt in node.body:
            dump_ast(stmt, indent + 2)
    elif isinstance(node, If):
        print(f"{prefix}If")
        print(f"{'  ' * (indent + 1)}Condition:")
        dump_ast(node.condition, indent + 2)
        print(f"{'  ' * (indent + 1)}Body:")
        for stmt in node.body:
            dump_ast(stmt, indent + 2)
        for cond, body in node.elifs:
            print(f"{'  ' * (indent + 1)}Elif:")
            dump_ast(cond, indent + 2)
            for stmt in body:
                dump_ast(stmt, indent + 2)
        if node.else_body:
            print(f"{'  ' * (indent + 1)}Else:")
            for stmt in node.else_body:
                dump_ast(stmt, indent + 2)
    elif isinstance(node, FuncDef):
        print(f"{prefix}FuncDef: {node.name}({', '.join(node.params)})")
        for stmt in node.body:
            dump_ast(stmt, indent + 1)
    elif isinstance(node, Return):
        print(f"{prefix}Return")
        dump_ast(node.value, indent + 1)
    elif isinstance(node, Break):
        print(f"{prefix}Break")
    elif isinstance(node, Continue):
        print(f"{prefix}Continue")
    elif isinstance(node, ExprStmt):
        print(f"{prefix}ExprStmt")
        dump_ast(node.expr, indent + 1)
    else:
        print(f"{prefix}Unknown node: {type(node)}")
        
# mas_ast.py — Add at the bottom

class MASObject:
    """Base class for all MAS runtime objects (for future GC)."""
    def __init__(self):
        self.refcount = 1

    def incref(self):
        self.refcount += 1

    def decref(self):
        self.refcount -= 1
        if self.refcount == 0:
            self._free()

    def _free(self):
        pass  # Override in subclasses

class MASNumber(MASObject):
    def __init__(self, value):
        super().__init__()
        self.value = value
    def __repr__(self): return str(self.value)

class MASString(MASObject):
    def __init__(self, value):
        super().__init__()
        self.value = value
    def __repr__(self): return f'"{self.value}"'

class MASBoolean(MASObject):
    def __init__(self, value):
        super().__init__()
        self.value = value
    def __repr__(self): return 'true' if self.value else 'false'

class MASNull(MASObject):
    def __repr__(self): return 'null'

class MASList(MASObject):
    def __init__(self, items):
        super().__init__()
        self.items = items  # list of MASObject
        for item in self.items:
            item.incref()
    def __repr__(self): return '[' + ', '.join(repr(x) for x in self.items) + ']'
    def _free(self):
        for item in self.items:
            item.decref()

class MASDict(MASObject):
    def __init__(self, pairs):
        super().__init__()
        # pairs: list of (key: MASObject, value: MASObject)
        self.pairs = pairs
        for k, v in self.pairs:
            k.incref()
            v.incref()
    def __repr__(self):
        items = [f"{k}: {v}" for k, v in self.pairs]
        return '{' + ', '.join(items) + '}'
    def _free(self):
        for k, v in self.pairs:
            k.decref()
            v.decref()