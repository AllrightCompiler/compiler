import sys

reg_map = {}
n_reg = 0
lib_funcs = [
    ["declare i32 @getint()"],
    ["declare i32 @getch()"],
    ["declare float @getfloat()"],
    ["declare i32 @putint(i32)"],
    ["declare i32 @putch(i32)"],
    ["declare float @putfloat(float)"],
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

def replace_br(line):
    if len(line) != 9:
        return line
    if line[0] != "br":
        return line
    assert line[1] == "i32"
    line[1] = "i1"
    return line

def rename_regs(s: str):
    global reg_map
    global n_reg
    if s.startswith("%") and s[1:].isdigit():
        reg_id = int(s[1:])
        s = "%" + str(reg_map[reg_id])
    return s

def collect_regs(s: str):
    global reg_map
    global n_reg
    if s.startswith("%") and s[1:].isdigit():
        reg_id = int(s[1:])
        if reg_id not in reg_map:
            reg_map[reg_id] = n_reg
            n_reg = n_reg + 1
    return s

def translate(input_path, output_path):
    content = open(input_path).readlines()
    content = [line.replace(",", " , ") for line in content]
    content = [line.replace("[", " [ ") for line in content]
    content = [line.replace("]", " ] ") for line in content]
    content = [line.split() for line in content]
    content = [replace_loadimm(line) for line in content]
    content = [replace_cmp(line) for line in content]
    content = [replace_br(line) for line in content]
    for line in content:
        if line:
            collect_regs(line[0])
    content = [[rename_regs(s) for s in line] for line in content]
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