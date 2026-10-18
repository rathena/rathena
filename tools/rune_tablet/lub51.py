#!/usr/bin/env python3
"""Minimal Lua 5.1 bytecode loader and VM for Ragnarok client tables (.lub).

Runs main chunks that build tables (NEWTABLE/SETTABLE/SETLIST/SETGLOBAL,
arithmetic, concat, simple helper calls) without a Lua runtime, so compiled
System/Rune lubs can be mined into db/<mode>/rune_tablet_*.yml.
Strings are decoded from CP949, or from the LUB_ENCODING environment
variable (cp950 for TWRoZ).

Usage:
    python tools/rune_tablet/lub51.py <file.lub>        dump globals as JSON
    python tools/rune_tablet/lub51.py <file.lub> -d     disassemble

See doc/rune_tablet.txt (Pre-renewal data).
"""
import struct, sys


class LuaFunction:
    def __init__(self, proto, upvals):
        self.proto = proto
        self.upvals = upvals

    def __repr__(self):
        return "<function %s:%d>" % (self.proto["source"], self.proto["line"])


class Reader:
    def __init__(self, data):
        self.d, self.p = data, 0

    def take(self, n):
        v = self.d[self.p:self.p + n]
        self.p += n
        return v

    def byte(self):
        return self.take(1)[0]

    def int(self):
        return struct.unpack("<i", self.take(self.sint))[0]

    def size_t(self):
        return struct.unpack("<I" if self.ssize == 4 else "<Q", self.take(self.ssize))[0]

    def num(self):
        return struct.unpack("<d", self.take(8))[0]

    def string(self):
        n = self.size_t()
        if n == 0:
            return None
        return self.take(n)[:-1]


#: Client text encoding: cp949 for kRO, cp950 (Big5) for TWRoZ. Override with LUB_ENCODING.
ENCODING = __import__("os").environ.get("LUB_ENCODING", "cp949")


def decode(b):
    return b.decode(ENCODING, errors="replace")


def load_proto(r):
    src = r.string()
    f = {"source": decode(src) if src else "?", "line": r.int()}
    r.int()
    f["nups"], f["nparams"], f["vararg"], f["maxstack"] = r.byte(), r.byte(), r.byte(), r.byte()
    f["code"] = [struct.unpack("<I", r.take(4))[0] for _ in range(r.int())]
    consts = []
    for _ in range(r.int()):
        t = r.byte()
        if t == 0:
            consts.append(None)
        elif t == 1:
            consts.append(bool(r.byte()))
        elif t == 3:
            v = r.num()
            consts.append(int(v) if v == int(v) else v)
        elif t == 4:
            s = r.string()
            consts.append(decode(s) if s is not None else "")
        else:
            raise ValueError("bad constant type %d" % t)
    f["consts"] = consts
    f["protos"] = [load_proto(r) for _ in range(r.int())]
    for _ in range(r.int()):
        r.int()
    for _ in range(r.int()):
        r.string(); r.int(); r.int()
    for _ in range(r.int()):
        r.string()
    return f


def load_chunk(data):
    r = Reader(data)
    if r.take(4) != b"\x1bLua" or r.byte() != 0x51:
        raise ValueError("not a Lua 5.1 chunk")
    r.byte()  # format
    if r.byte() != 1:
        raise ValueError("big endian chunk")
    r.sint, r.ssize, sinst, snum, integral = r.byte(), r.byte(), r.byte(), r.byte(), r.byte()
    assert sinst == 4 and snum == 8 and integral == 0
    return load_proto(r)


def norm_key(k):
    if isinstance(k, float) and k == int(k):
        return int(k)
    return k


class VM:
    def __init__(self, globals_=None, builtins=None):
        self.G = globals_ if globals_ is not None else {}
        self.builtins = builtins or {}

    def call(self, fn, args):
        if callable(fn) and not isinstance(fn, LuaFunction):
            r = fn(*args)
            return list(r) if isinstance(r, tuple) else ([] if r is None else [r])
        return self.run(fn.proto, args, fn.upvals)

    def run(self, f, args=(), upvals=()):
        code, K = f["code"], f["consts"]
        R = [None] * (f["maxstack"] + 256)
        for i, a in enumerate(args[:f["nparams"]]):
            R[i] = a
        pc, top = 0, 0

        def RK(x):
            return K[x - 256] if x >= 256 else R[x]

        while pc < len(code):
            i = code[pc]; pc += 1
            op, A = i & 0x3F, (i >> 6) & 0xFF
            C, B = (i >> 14) & 0x1FF, (i >> 23) & 0x1FF
            Bx = i >> 14; sBx = Bx - 131071
            if op == 0: R[A] = R[B]
            elif op == 1: R[A] = K[Bx]
            elif op == 2:
                R[A] = bool(B)
                if C: pc += 1
            elif op == 3:
                for j in range(A, B + 1): R[j] = None
            elif op == 4: R[A] = upvals[B][0][upvals[B][1]]
            elif op == 5:
                name = K[Bx]
                R[A] = self.G.get(name, self.builtins.get(name))
            elif op == 6:
                t = R[B]
                R[A] = None if t is None else t.get(norm_key(RK(C)))
            elif op == 7: self.G[K[Bx]] = R[A]
            elif op == 8: upvals[B][0][upvals[B][1]] = R[A]
            elif op == 9: R[A][norm_key(RK(B))] = RK(C)
            elif op == 10: R[A] = {}
            elif op == 11:
                R[A + 1] = R[B]; R[A] = R[B].get(norm_key(RK(C)))
            elif 12 <= op <= 17:
                x, y = RK(B), RK(C)
                R[A] = {12: lambda: x + y, 13: lambda: x - y, 14: lambda: x * y, 15: lambda: x / y,
                        16: lambda: x % y, 17: lambda: x ** y}[op]()
                if isinstance(R[A], float) and R[A] == int(R[A]): R[A] = int(R[A])
            elif op == 18: R[A] = -R[B]
            elif op == 19: R[A] = not (R[B] is not None and R[B] is not False)
            elif op == 20:
                t = R[B]
                if isinstance(t, str): R[A] = len(t)
                else:
                    n = 0
                    while (n + 1) in t: n += 1
                    R[A] = n
            elif op == 21:
                R[A] = "".join(str(R[j]) if not isinstance(R[j], str) else R[j] for j in range(B, C + 1))
            elif op == 22: pc += sBx
            elif op in (23, 24, 25):
                x, y = RK(B), RK(C)
                res = {23: lambda: x == y, 24: lambda: x < y, 25: lambda: x <= y}[op]()
                if res != bool(A): pc += 1
            elif op == 26:
                truthy = R[A] is not None and R[A] is not False
                if truthy != bool(C): pc += 1
            elif op == 27:
                truthy = R[B] is not None and R[B] is not False
                if truthy == bool(C): R[A] = R[B]
                else: pc += 1
            elif op in (28, 29):
                fn = R[A]
                nargs = (B - 1) if B else (top - A - 1)
                callargs = R[A + 1:A + 1 + nargs]
                if fn is None:
                    res = []
                else:
                    res = self.call(fn, callargs)
                if op == 29:
                    return res
                if C == 0:
                    for j, v in enumerate(res): R[A + j] = v
                    top = A + len(res)
                else:
                    for j in range(C - 1): R[A + j] = res[j] if j < len(res) else None
            elif op == 30:
                if B == 0: return R[A:top]
                return R[A:A + B - 1]
            elif op == 31:
                R[A] += R[A + 2]
                if (R[A + 2] > 0 and R[A] <= R[A + 1]) or (R[A + 2] <= 0 and R[A] >= R[A + 1]):
                    pc += sBx; R[A + 3] = R[A]
            elif op == 32:
                R[A] -= R[A + 2]; pc += sBx
            elif op == 33:
                res = self.call(R[A], [R[A + 1], R[A + 2]])
                for j in range(C): R[A + 3 + j] = res[j] if j < len(res) else None
                if R[A + 3] is not None:
                    R[A + 2] = R[A + 3]
                else:
                    pc += 1
            elif op == 34:
                if C == 0:
                    C = code[pc]; pc += 1
                n = B if B else (top - A - 1)
                for j in range(1, n + 1):
                    R[A][(C - 1) * 50 + j] = R[A + j]
            elif op == 35: pass
            elif op == 36:
                p = f["protos"][Bx]
                ups = []
                for _ in range(p["nups"]):
                    pi = code[pc]; pc += 1
                    pop, pB = pi & 0x3F, (pi >> 23) & 0x1FF
                    if pop == 0: ups.append((R, pB))
                    else: ups.append(upvals[pB])
                R[A] = LuaFunction(p, ups)
            elif op == 37:
                n = B - 1 if B else 0
                for j in range(n): R[A + j] = None
            else:
                raise ValueError("opcode %d" % op)
        return []


def lua_pairs(t):
    keys = list(t.keys())

    def nxt(tbl, k):
        idx = 0 if k is None else keys.index(k) + 1
        return (keys[idx], tbl[keys[idx]]) if idx < len(keys) else (None,)
    return nxt, t, None


DEFAULT_BUILTINS = {
    "pairs": lua_pairs,
    "ipairs": lua_pairs,
    "tostring": lambda v: str(v),
    "tonumber": lambda v, b=10: int(v, b) if isinstance(v, str) else v,
}


def run_file(data, G=None, builtins=None):
    vm = VM(G, dict(DEFAULT_BUILTINS, **(builtins or {})))
    vm.run(load_chunk(data))
    return vm.G


def disasm(f, depth=0):
    names = ["MOVE", "LOADK", "LOADBOOL", "LOADNIL", "GETUPVAL", "GETGLOBAL", "GETTABLE", "SETGLOBAL",
             "SETUPVAL", "SETTABLE", "NEWTABLE", "SELF", "ADD", "SUB", "MUL", "DIV", "MOD", "POW", "UNM",
             "NOT", "LEN", "CONCAT", "JMP", "EQ", "LT", "LE", "TEST", "TESTSET", "CALL", "TAILCALL", "RETURN",
             "FORLOOP", "FORPREP", "TFORLOOP", "SETLIST", "CLOSE", "CLOSURE", "VARARG"]
    out = ["%sfunction %s:%d params=%d consts=%r" % ("  " * depth, f["source"], f["line"], f["nparams"], f["consts"][:40])]
    for n, i in enumerate(f["code"]):
        op = i & 0x3F
        out.append("%s  %3d %-9s A=%d B=%d C=%d Bx=%d" % ("  " * depth, n, names[op], (i >> 6) & 0xFF,
                                                          (i >> 23) & 0x1FF, (i >> 14) & 0x1FF, i >> 14))
    for p in f["protos"]:
        out.append(disasm(p, depth + 1))
    return "\n".join(out)


if __name__ == "__main__":
    import json
    data = open(sys.argv[1], "rb").read()
    if len(sys.argv) > 2 and sys.argv[2] == "-d":
        print(disasm(load_chunk(data)))
    else:
        G = run_file(data)
        print(json.dumps({k: v for k, v in G.items() if not isinstance(v, LuaFunction)},
                         ensure_ascii=False, indent=1, default=str)[:6000])
        print("functions:", [k for k, v in G.items() if isinstance(v, LuaFunction)])
