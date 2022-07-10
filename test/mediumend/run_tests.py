import os
import sys
import subprocess

TIMEOUT = 10

def get_testcases(test_dir):
    testcases = []
    for file in os.listdir(test_dir):
        if os.path.splitext(file)[1] == ".sy":
            testcases.append(os.path.splitext(file)[0])
    testcases.sort()
    return testcases

def run_test(compiler_path, converter_path, lib_path, test_dir, test_name):
    code_path = test_dir + test_name + ".sy"
    input_path = test_dir + test_name + ".in"
    output_path = test_dir + test_name + ".tmp"
    ir_path = test_dir + test_name + ".ir"
    ll_path = test_dir + test_name + ".ll"
    obj_path = test_dir + test_name + ".o"
    exe_path = test_dir + test_name
    ans_path = test_dir + test_name + ".out"
    print("Running test {}: ".format(test_name), end="")

    command = f'{compiler_path} --ir -O2 {code_path}'
    proc = subprocess.Popen(command, stdout=open(ir_path, "w"), stderr=open("/dev/null", "w"), shell=True)
    try:
        proc.wait(TIMEOUT)
    except subprocess.TimeoutExpired:
        proc.kill()
        print('\033[0;31mCompiler TLE\033[0m')
        return False
    if proc.returncode:
        print('\033[0;31mCompiler Error\033[0m')
        return False
    
    command = f'python3 {converter_path} {ir_path} {ll_path}'
    proc = subprocess.Popen(command, stderr=open("/dev/null", "w"), shell=True)
    proc.wait()
    if proc.returncode:
        print('\033[0;31mTranslator Error\033[0m')
        return False

    command = f'llc {ll_path} -filetype=obj -o {obj_path}'
    proc = subprocess.Popen(command, stderr=open("/dev/null", "w"), shell=True)
    proc.wait()
    if proc.returncode:
        print('\033[0;31mllc Error\033[0m')
        return False

    command = f'gcc {obj_path} {lib_path} -o {exe_path}'
    proc = subprocess.Popen(command, stderr=open("/dev/null", "w"), shell=True)
    proc.wait()
    if proc.returncode:
        print('\033[0;31mLinker Error\033[0m')
        return False
    
    command = f'{exe_path}'
    if (os.path.exists(input_path)):
        proc = subprocess.Popen(command, stdin=open(input_path, "r"), stdout=open(output_path, "w",), stderr=open("/dev/null", "w"), shell=True)
    else:
        proc = subprocess.Popen(command, stdout=open(output_path, "w"), stderr=open("/dev/null", "w"), shell=True)
    try:
        proc.wait(TIMEOUT)
        with open(output_path, "a") as f:
            f.write("\n"+str(proc.returncode))
    except subprocess.TimeoutExpired:
        proc.kill()
        print('\033[0;31mTLE\033[0m')
        return False
    

    command = f'diff -b -B {output_path} {ans_path}'
    proc = subprocess.Popen(command, stdout=open("/dev/null", "w"), shell=True)
    try:
        proc.wait(TIMEOUT)
    except subprocess.TimeoutExpired:
        proc.kill()
        print('\033[0;31mDIFF TLE\033[0m')
        return False

    if proc.returncode:
        print('\033[0;31mWrong Answer\033[0m')
        return False
    else:
        print('\033[0;32mPass\033[0m')
        return True

if __name__ == '__main__':
    if len(sys.argv) != 5:
        print("usage: python3 run_test.py <compiler> <ir_to_llvm.py> <libsysy_x86.a> <test_dir>")
        exit(1)
    compiler_path = sys.argv[1]
    converter_path = sys.argv[2]
    lib_path = sys.argv[3]
    test_dir = sys.argv[4]

    testcases = get_testcases(test_dir)
    num_tests = len(testcases)
    pass_tests = 0
    for test_name in testcases:
        if run_test(compiler_path, converter_path, lib_path, test_dir, test_name):
            pass_tests = pass_tests + 1
    if pass_tests == num_tests:
        print(f'\033[0;32m{pass_tests}/{num_tests} tests Passed\033[0m')
    else:
        print(f'\033[0;31m{pass_tests}/{num_tests} tests Passed\033[0m')
    