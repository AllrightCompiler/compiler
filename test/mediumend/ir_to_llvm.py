import sys
import re

reg_map = {}
addr_map = {}
n_reg = 0
lib_funcs = [
    ["declare i32 @getint()"],
    ["declare i32 @getch()"],
    ["declare float @getfloat()"],
    ["declare void @putint(i32)"],
    ["declare void @putch(i32)"],
    ["declare void @putfloat(float)"],
    []
]

def replace_loadimm(line):
    if len(line) != 5:
        return line
    if line[2] != "loadimm":
        return line
    return [line[0], line[1], "add", line[3], "0", ",", line[4]]

def replace_cmp(line):
    if len(line) != 8:
        return line
    if line[2] != "cmp":
        return line
    if line[4] == "i32":
        line[2] = "icmp"
        if line[3] == "ge" or line[3] == "le" or line[3] == "gt" or line[3] == "lt":
            line[3] = "s" + line[3]
    elif line[4] == "float":
        line[2] = "fcmp"
        # TODO
    else:
        assert False
    return line

def replace_div_mod(line):
    if len(line) != 7:
        return line
    if line[2] != "div" and line[2] != "mod":
        return line
    if line[2] == "mod":
        line[2] = "rem"
    if line[3] == "i32":
        line[2] = "s" + line[2]
    elif line[3] == "float":
        line[2] = "f" + line[2]
    else:
        assert False
    return line

def replace_br(line):
    if len(line) != 9:
        return line
    if line[0] != "br":
        return line
    assert line[1] == "i32"
    line[1] = "i1"
    return line

def replace_funcall(line):
    if(len(line) < 3):
        return line
    if(line[2] != "call"):
        return line
    if(line[3] == "void"):
        return line[2::]
    return line

def collect_loadaddr(line):
    global addr_map
    if len(line) != 4:
        return line
    if line[2] != "loadaddr":
        return line
    addr_map[int(line[0][1:])] = line[3]
    return []

def rename_regs(mp, s: str):
    if s.startswith("%") and s[1:].isdigit():
        reg_id = int(s[1:])
        if reg_id in mp:
            s = mp[reg_id]
            return s
    return s

def collect_regs(s: str):
    global reg_map
    global n_reg
    if s.startswith("%") and s[1:].isdigit():
        reg_id = int(s[1:])
        if reg_id not in reg_map:
            reg_map[reg_id] = "%" + str(n_reg)
            n_reg = n_reg + 1
    return s

def translate_func(content):
    global reg_map
    global addr_map
    global n_reg
    addr_map = {}
    for idx in range(len(content)):
        content[idx] = collect_loadaddr(content[idx])
    content = [[rename_regs(addr_map, s) for s in line] for line in content]
    reg_map = {}
    n_reg = 0
    for param in content[0]:
        collect_regs(param)
    for line in content:
        if line:
            collect_regs(line[0])
    content = [[rename_regs(reg_map, s) for s in line] for line in content]
    return content

def translate(input_path, output_path):
    global reg_map
    global addr_map
    global n_reg
    content = open(input_path).readlines()
    content = [line.replace(",", " , ") for line in content]
    content = [line.replace("[", " [ ") for line in content]
    content = [line.replace("]", " ] ") for line in content]
    content = [line.replace("(", " ( ") for line in content]
    content = [line.replace(")", " ) ") for line in content]
    content = [line.split() for line in content]
    content = [replace_loadimm(line) for line in content]
    content = [replace_cmp(line) for line in content]
    content = [replace_div_mod(line) for line in content]
    content = [replace_br(line) for line in content]
    content = [replace_funcall(line) for line in content]
    begin_idx = 0
    for idx in range(len(content)):
        line = content[idx]
        if line:
            if line[0] == "define": # new func
                begin_idx = idx
            if line[0] == "}": # end func
                content[begin_idx : idx] = translate_func(content[begin_idx : idx])
    global lib_funcs
    content = lib_funcs + content

    content = [" ".join(line) + "\n" for line in content]
    open(output_path, "w").writelines(content)

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("usage: python3 ir_to_llvm.py <input_ir.ir> <output.ll>")
        exit(1)
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    translate(input_file, output_file)