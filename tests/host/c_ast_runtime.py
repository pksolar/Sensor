"""Small C99 AST interpreter for the project's hardware-independent production code.

It intentionally supports only the constructs used by measurement_core.c,
protocol_codec.c and persistent_codec.c. Tests execute those production function
bodies directly; algorithm formulas are not duplicated in Python.
"""

from __future__ import annotations

import math
import re
import struct
from pathlib import Path

from pycparser import c_ast, c_parser


class ReturnSignal(Exception):
    def __init__(self, value):
        super().__init__()
        self.value = value


class ArrayPointer:
    def __init__(self, array, offset=0):
        if isinstance(array, ArrayPointer):
            self.array = array.array
            self.offset = array.offset + offset
        else:
            self.array = array
            self.offset = offset

    def __getitem__(self, index):
        return self.array[self.offset + index]

    def __setitem__(self, index, value):
        self.array[self.offset + index] = value


def _f32(value):
    return struct.unpack("<f", struct.pack("<f", float(value)))[0]


def _signed(value, bits):
    value &= (1 << bits) - 1
    sign = 1 << (bits - 1)
    return value - (1 << bits) if value & sign else value


def _type_name(node):
    if isinstance(node, c_ast.Decl):
        return _type_name(node.type)
    if isinstance(node, c_ast.Typename):
        return _type_name(node.type)
    if isinstance(node, c_ast.TypeDecl):
        return " ".join(node.type.names)
    if isinstance(node, c_ast.IdentifierType):
        return " ".join(node.names)
    if isinstance(node, c_ast.PtrDecl):
        return _type_name(node.type) + " *"
    if isinstance(node, c_ast.ArrayDecl):
        return _type_name(node.type) + " []"
    if isinstance(node, c_ast.FuncDecl):
        return _type_name(node.type)
    raise TypeError(f"unsupported type node: {type(node).__name__}")


def _cast(value, c_type):
    c_type = c_type.replace("const ", "").strip()
    if c_type.endswith("*") or c_type.endswith("[]"):
        return value
    if c_type in {"float"}:
        return _f32(value)
    if c_type in {"double"}:
        return float(value)
    if c_type in {"unsigned char"}:
        return int(value) & 0xFF
    if c_type in {"char", "signed char"}:
        return _signed(int(value), 8)
    if c_type in {"unsigned short", "unsigned short int"}:
        return int(value) & 0xFFFF
    if c_type in {"short", "short int", "signed short", "signed short int"}:
        return _signed(int(value), 16)
    if c_type in {"unsigned int", "unsigned long", "unsigned long int"}:
        return int(value) & 0xFFFFFFFF
    if c_type in {"int", "signed", "signed int", "long", "long int", "signed long"}:
        return _signed(int(value), 32)
    if c_type == "void":
        return None
    raise TypeError(f"unsupported cast: {c_type}")


class Environment:
    def __init__(self):
        self.values = {}
        self.types = {}

    def declare(self, name, c_type, value=0):
        self.types[name] = c_type
        self.values[name] = _cast(value, c_type)

    def assign(self, name, value):
        self.values[name] = _cast(value, self.types[name])
        return self.values[name]


class CModule:
    def __init__(self, paths):
        source = "\n".join(self._clean_source(Path(path).read_text(encoding="utf-8")) for path in paths)
        self._parse(source)

    @classmethod
    def from_source(cls, source):
        module = cls.__new__(cls)
        module._parse(module._clean_source(source))
        return module

    def _parse(self, source):
        ast = c_parser.CParser().parse(source)
        self.functions = {
            item.decl.name: item
            for item in ast.ext
            if isinstance(item, c_ast.FuncDef)
        }

    @staticmethod
    def _clean_source(source):
        source = re.sub(r"/\*.*?\*/", "", source, flags=re.S)
        source = re.sub(r"//.*", "", source)
        macros = {}
        for match in re.finditer(r"^\s*#define\s+(\w+)\s+([^\r\n]+)$", source, flags=re.M):
            name, value = match.groups()
            if "(" not in name and re.fullmatch(r"[()xXa-fA-F0-9uUlL+\-\s]+", value.strip()):
                macros[name] = value.strip()
        source = re.sub(r"^\s*#.*$", "", source, flags=re.M)
        for name, value in macros.items():
            source = re.sub(rf"\b{re.escape(name)}\b", f"({value})", source)
        return source

    def call(self, name, *arguments):
        function = self.functions[name]
        env = Environment()
        params = function.decl.type.args.params if function.decl.type.args else []
        if len(params) != len(arguments):
            raise TypeError(f"{name} expects {len(params)} arguments, got {len(arguments)}")
        for param, argument in zip(params, arguments):
            env.declare(param.name, _type_name(param), argument)
        try:
            self._statement(function.body, env)
        except ReturnSignal as signal:
            return _cast(signal.value, _type_name(function.decl.type.type))
        return None

    def _statement(self, node, env):
        if node is None:
            return
        if isinstance(node, c_ast.Compound):
            for item in node.block_items or []:
                self._statement(item, env)
            return
        if isinstance(node, c_ast.Decl):
            c_type = _type_name(node)
            if isinstance(node.type, c_ast.ArrayDecl):
                size = int(self._expr(node.type.dim, env))
                element_type = _type_name(node.type.type)
                values = [_cast(0, element_type) for _ in range(size)]
                if isinstance(node.init, c_ast.InitList):
                    for index, expression in enumerate(node.init.exprs):
                        values[index] = _cast(self._expr(expression, env), element_type)
                env.types[node.name] = element_type + " []"
                env.values[node.name] = values
            else:
                value = self._expr(node.init, env) if node.init is not None else 0
                env.declare(node.name, c_type, value)
            return
        if isinstance(node, c_ast.DeclList):
            for declaration in node.decls:
                self._statement(declaration, env)
            return
        if isinstance(node, c_ast.Assignment):
            self._assignment(node, env)
            return
        if isinstance(node, c_ast.UnaryOp):
            self._expr(node, env)
            return
        if isinstance(node, c_ast.FuncCall):
            self._expr(node, env)
            return
        if isinstance(node, c_ast.If):
            branch = node.iftrue if self._expr(node.cond, env) else node.iffalse
            self._statement(branch, env)
            return
        if isinstance(node, c_ast.For):
            self._statement(node.init, env)
            guard = 0
            while self._expr(node.cond, env):
                self._statement(node.stmt, env)
                self._expr(node.next, env)
                guard += 1
                if guard > 100000:
                    raise RuntimeError("loop guard exceeded")
            return
        if isinstance(node, c_ast.Return):
            raise ReturnSignal(self._expr(node.expr, env) if node.expr is not None else None)
        if isinstance(node, c_ast.EmptyStatement):
            return
        raise TypeError(f"unsupported statement: {type(node).__name__}")

    def _expr(self, node, env):
        if node is None:
            return None
        if isinstance(node, c_ast.Constant):
            text = node.value
            if node.type in {"float", "double"} or any(ch in text for ch in ".eE"):
                return float(re.sub(r"[fFlL]+$", "", text))
            return int(re.sub(r"[uUlL]+$", "", text), 0)
        if isinstance(node, c_ast.ID):
            return env.values[node.name]
        if isinstance(node, c_ast.ArrayRef):
            array = self._expr(node.name, env)
            return array[int(self._expr(node.subscript, env))]
        if isinstance(node, c_ast.Cast):
            return _cast(self._expr(node.expr, env), _type_name(node.to_type))
        if isinstance(node, c_ast.BinaryOp):
            if node.op == "&&":
                return int(bool(self._expr(node.left, env)) and bool(self._expr(node.right, env)))
            if node.op == "||":
                return int(bool(self._expr(node.left, env)) or bool(self._expr(node.right, env)))
            left = self._expr(node.left, env)
            right = self._expr(node.right, env)
            return self._binary(node.op, left, right)
        if isinstance(node, c_ast.UnaryOp):
            if node.op in {"p++", "++", "p--", "--"}:
                delta = 1 if "+" in node.op else -1
                old = self._expr(node.expr, env)
                self._set_lvalue(node.expr, old + delta, env)
                return old if node.op.startswith("p") else old + delta
            value = self._expr(node.expr, env)
            if node.op == "&" and isinstance(node.expr, c_ast.ArrayRef):
                array = self._expr(node.expr.name, env)
                index = int(self._expr(node.expr.subscript, env))
                return ArrayPointer(array, index)
            if node.op == "+":
                return value
            if node.op == "-":
                return -value
            if node.op == "!":
                return int(not value)
            if node.op == "~":
                return ~int(value)
            raise TypeError(f"unsupported unary op: {node.op}")
        if isinstance(node, c_ast.Assignment):
            return self._assignment(node, env)
        if isinstance(node, c_ast.FuncCall):
            arguments = [] if node.args is None else [self._expr(item, env) for item in node.args.exprs]
            if node.name.name == "fabs":
                return math.fabs(arguments[0])
            return self.call(node.name.name, *arguments)
        if isinstance(node, c_ast.ExprList):
            value = None
            for expression in node.exprs:
                value = self._expr(expression, env)
            return value
        raise TypeError(f"unsupported expression: {type(node).__name__}")

    @staticmethod
    def _binary(operator, left, right):
        if operator == "+":
            return left + right
        if operator == "-":
            return left - right
        if operator == "*":
            return left * right
        if operator == "/":
            if isinstance(left, int) and isinstance(right, int):
                return int(left / right)
            return left / right
        if operator == "%":
            return left % right
        if operator == "<<":
            return int(left) << int(right)
        if operator == ">>":
            return int(left) >> int(right)
        if operator == "&":
            return int(left) & int(right)
        if operator == "|":
            return int(left) | int(right)
        if operator == "^":
            return int(left) ^ int(right)
        if operator == "<":
            return int(left < right)
        if operator == "<=":
            return int(left <= right)
        if operator == ">":
            return int(left > right)
        if operator == ">=":
            return int(left >= right)
        if operator == "==":
            return int(left == right)
        if operator == "!=":
            return int(left != right)
        raise TypeError(f"unsupported binary op: {operator}")

    def _assignment(self, node, env):
        right = self._expr(node.rvalue, env)
        if node.op != "=":
            left = self._expr(node.lvalue, env)
            right = self._binary(node.op[:-1], left, right)
        return self._set_lvalue(node.lvalue, right, env)

    def _set_lvalue(self, node, value, env):
        if isinstance(node, c_ast.ID):
            return env.assign(node.name, value)
        if isinstance(node, c_ast.ArrayRef):
            array = self._expr(node.name, env)
            index = int(self._expr(node.subscript, env))
            array[index] = value
            return value
        raise TypeError(f"unsupported lvalue: {type(node).__name__}")
