# mas_interpreter.py

from mas_ast import *

class ReturnException(Exception):
    def __init__(self, value):
        self.value = value

class StopException(Exception):
    pass

class NextException(Exception):
    pass

class MASInterpreter:
    
    def __init__(self):
        self.globals = {}
        self.locals = [{}]  # stack of scopes

    # In MASInterpreter, add to top of visit methods:

    def visit_Assign(self, node):
        print(f"📝 Assign: {node.name}")
        value = self.visit(node.value)
        self.current_scope()[node.name] = value

    def visit_Loop(self, node):
        print("🔁 Entering loop")
        # ... rest of loop logic

    def visit_Each(self, node):
        print("🔄 Entering each loop")
        # ... rest

    def visit_If(self, node):
        print("❓ Evaluating if")
        # ... rest
    def current_scope(self):
        return self.locals[-1]

    def visit(self, node):
        method_name = f'visit_{type(node).__name__}'
        visitor = getattr(self, method_name, self.generic_visit)
        return visitor(node)

    def generic_visit(self, node):
        raise Exception(f'No visit_{type(node).__name__} method')

    # --- Program ---
    def visit_Program(self, node):
        for stmt in node.statements:
            self.visit(stmt)

    # --- Literals ---
    def visit_Number(self, node):
        return MASNumber(node.value)

    def visit_String(self, node):
        return MASString(node.value)

    def visit_Boolean(self, node):
        return MASBoolean(node.value)

    def visit_Null(self, node):
        return MASNull()

    def visit_Var(self, node):
        name = node.name
        for scope in reversed(self.locals):
            if name in scope:
                val = scope[name]
                print(f"🔍 Reading var '{name}' = {val} (type: {type(val)})")
                return val
        if name in self.globals:
            return self.globals[name]
        raise NameError(f"Variable '{name}' not defined")

    # --- Lists & Dicts ---
    def visit_List(self, node):
        items = [self.visit(elem) for elem in node.elements]
        return MASList(items)

    def visit_Dict(self, node):
        pairs = []
        for key_expr, val_expr in node.pairs:
            key = self.visit(key_expr)
            val = self.visit(val_expr)
            pairs.append((key, val))
        return MASDict(pairs)

    # --- Binary Ops ---
    def visit_BinOp(self, node):
        left = self.visit(node.left)
        right = self.visit(node.right)

        if isinstance(left, MASNumber) and isinstance(right, MASNumber):
            if node.op == '+': return MASNumber(left.value + right.value)
            if node.op == '-': return MASNumber(left.value - right.value)
            if node.op == '*': return MASNumber(left.value * right.value)
            if node.op == '/': return MASNumber(left.value / right.value)
            if node.op == '==': return MASBoolean(left.value == right.value)
            if node.op == '!=': return MASBoolean(left.value != right.value)
            if node.op == '<': return MASBoolean(left.value < right.value)
            if node.op == '<=': return MASBoolean(left.value <= right.value)
            if node.op == '>': return MASBoolean(left.value > right.value)
            if node.op == '>=': return MASBoolean(left.value >= right.value)

        raise TypeError(f"Unsupported operation {node.op} between {type(left)} and {type(right)}")

    def visit_UnaryOp(self, node):
        operand = self.visit(node.operand)
        if node.op == '-' and isinstance(operand, MASNumber):
            return MASNumber(-operand.value)
        raise TypeError(f"Unsupported unary op {node.op}")

        # --- Assign ---
    def visit_Assign(self, node):
        value = self.visit(node.value)
        print(f"💾 Assigning '{node.name}' = {value}")
        self.current_scope()[node.name] = value

    # --- Control Flow ---
    def visit_If(self, node):
        cond = self.visit(node.condition)
        if not isinstance(cond, MASBoolean):
            raise TypeError("Condition must be boolean")
        if cond.value:
            for stmt in node.body:
                self.visit(stmt)
        elif node.elifs:
            for elif_cond, elif_body in node.elifs:
                cond = self.visit(elif_cond)
                if isinstance(cond, MASBoolean) and cond.value:
                    for stmt in elif_body:
                        self.visit(stmt)
                    return
            if node.else_body:
                for stmt in node.else_body:
                    self.visit(stmt)
        elif node.else_body:
            for stmt in node.else_body:
                self.visit(stmt)

    def visit_Each(self, node):
        iterable = self.visit(node.iterable)
        if not isinstance(iterable, MASList):
            raise TypeError("Can only iterate over lists")
        target_var = node.target.name
        for item in iterable.items:
            self.current_scope()[target_var] = item
            try:
                for stmt in node.body:
                    self.visit(stmt)
            except StopException:
                print("🛑 Stop caught in 'each' — breaking")
                break
            except NextException:
                print("⏭️ Next caught — continuing")
                continue

    def visit_Loop(self, node):
        iteration = 0
        while True:
            cond = self.visit(node.condition)
            print(f"🔁 Loop check #{iteration}: condition = {cond}")
            if not isinstance(cond, MASBoolean):
                raise TypeError("Loop condition must be boolean")
            if not cond.value:
                print("🛑 Loop condition false — exiting")
                break
            print(f"▶️ Executing loop body (iteration {iteration})")
            try:
                for stmt in node.body:
                    self.visit(stmt)
            except StopException:
                print("🛑 Stop in loop — breaking")
                break
            except NextException:
                print("⏭️ Next in loop — continuing")
                continue
            iteration += 1
            if iteration > 20:  # safety
                print("⚠️ Infinite loop guard")
                break
    # --- Functions (simple, no params yet) ---
    def visit_FuncDef(self, node):
        # Store function AST in globals for now
        self.globals[node.name] = node

    def visit_Call(self, node):
        func_name = None
        if isinstance(node.func, Var):
            func_name = node.func.name
        else:
            raise TypeError("Only simple function calls supported")

        if func_name == 'print':
            print(" DEBUG: print called!")  # Should appear now
            # ... rest of print logic
            args = []
            for arg in node.args:
                mas_obj = self.visit(arg)
                if isinstance(mas_obj, MASString):
                    args.append(mas_obj.value)
                elif isinstance(mas_obj, MASNumber):
                    args.append(str(mas_obj.value))
                else:
                    args.append(repr(mas_obj))
            print(*args, flush=True)
            return MASNull()
        if func_name in self.globals:
            func_def = self.globals[func_name]
            if len(node.args) != len(func_def.params):
                raise TypeError(f"Function {func_name} expects {len(func_def.params)} args, got {len(node.args)}")
            
            # Create new local scope
            old_locals = self.locals
            self.locals = [{}]  # fresh local scope
            
            # Bind arguments
            for param, arg_expr in zip(func_def.params, node.args):
                arg_val = self.visit(arg_expr)
                self.locals[0][param] = arg_val
            
            # Execute function body
            result = MASNull()
            try:
                for stmt in func_def.body:
                    self.visit(stmt)
            except ReturnException as e:
                result = e.value
            finally:
                self.locals = old_locals  # restore outer scope
            
            return result
        else:
            raise NameError(f"Function '{func_name}' not defined")
   
    # --- Statements ---
    def visit_ExprStmt(self, node):
        self.visit(node.expr)

    def visit_Return(self, node):
        value = self.visit(node.value) if node.value else MASNull()
        raise ReturnException(value)

    def visit_Break(self, node):
        raise StopException()

    def visit_Continue(self, node):
        raise NextException()