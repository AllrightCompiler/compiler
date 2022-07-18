import os
import sys
import time
import subprocess
from junit_xml import TestSuite, TestCase

TIMEOUT = 30

def get_testcases(test_dir):
    testcases = []
    for file in os.listdir(test_dir):
        if os.path.splitext(file)[1] == ".sy":
            testcases.append(os.path.splitext(file)[0])
    testcases.sort()
    return testcases

def ret_err(test_name, timeStarted, msg):
    tc = TestCase(test_name, "mediumend-test", time.time() - timeStarted, "", "")
    tc.add_failure_info(msg)
    return tc, False

def run_test(compiler_path, converter_path, lib_path, test_dir, test_name):
    code_path = os.path.join(test_dir, test_name + ".sy")
    input_path = os.path.join(test_dir, test_name + ".in")
    output_path = os.path.join(test_dir, test_name + ".tmp")
    ir_path = os.path.join(test_dir, test_name + ".ir")
    ll_path = os.path.join(test_dir, test_name + ".ll")
    obj_path = os.path.join(test_dir, test_name + ".o")
    exe_path = os.path.join(test_dir, test_name)
    ans_path = os.path.join(test_dir, test_name + ".out")
    print("Running test {}: ".format(test_name), end="")

    command = f'{compiler_path} --ir -O2 {code_path}'
    timeStarted = time.time()
    proc = subprocess.Popen(command, stdout=open(ir_path, "w"), stderr=open("/dev/null", "w"), shell=True)
    try:
        proc.wait(TIMEOUT)
    except subprocess.TimeoutExpired:
        proc.kill()
        print('\033[0;31mCompiler TLE\033[0m')
        return ret_err(test_name, timeStarted, "Compiler TLE")
    if proc.returncode:
        print('\033[0;31mCompiler Error\033[0m')
        return ret_err(test_name, timeStarted, "Compiler Error")
    
    command = f'{compiler_path} --llvm -O2 {code_path}'
    timeStarted = time.time()
    proc = subprocess.Popen(command, stdout=open(ll_path, "w"), stderr=open("/dev/null", "w"), shell=True)
    try:
        proc.wait(TIMEOUT)
    except subprocess.TimeoutExpired:
        proc.kill()
        print('\033[0;31mTranslator TLE\033[0m')
        return ret_err(test_name, timeStarted, "Translator TLE")
    if proc.returncode:
        print('\033[0;31mTranslator Error\033[0m')
        return ret_err(test_name, timeStarted, "Translator Error")
    
    # command = f'python3 {converter_path} {ir_path} {ll_path}'
    # proc = subprocess.Popen(command, stderr=open("/dev/null", "w"), shell=True)
    # proc.wait()
    # if proc.returncode:
    #     print('\033[0;31mTranslator Error\033[0m')
    #     return False

    command = f'llc {ll_path} -filetype=obj -o {obj_path}'
    timeStarted = time.time()
    proc = subprocess.Popen(command, stderr=open("/dev/null", "w"), shell=True)
    try:
        proc.wait(TIMEOUT)
    except subprocess.TimeoutExpired:
        proc.kill()
        print('\033[0;31mllc TLE\033[0m')
        return ret_err(test_name, timeStarted, "llc TLE")
    if proc.returncode:
        print('\033[0;31mllc Error\033[0m')
        return ret_err(test_name, timeStarted, "llc Error")

    command = f'clang {obj_path} {lib_path} -o {exe_path}'
    timeStarted = time.time()
    proc = subprocess.Popen(command, stderr=open("/dev/null", "w"), shell=True)
    proc.wait()
    if proc.returncode:
        print('\033[0;31mLinker Error\033[0m')
        return ret_err(test_name, timeStarted, "Linker Error")
    
    command = f'{exe_path}'
    timeStarted = time.time()
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
        return ret_err(test_name, timeStarted, "TLE")
    

    command = f'diff -b -B {output_path} {ans_path}'
    timeStarted = time.time()
    proc = subprocess.Popen(command, stdout=open("/dev/null", "w"), shell=True)
    try:
        proc.wait(TIMEOUT)
    except subprocess.TimeoutExpired:
        proc.kill()
        print('\033[0;31mDIFF TLE\033[0m')
        return ret_err(test_name, timeStarted, "DIFF TLE")

    if proc.returncode:
        print('\033[0;31mWrong Answer\033[0m')
        return ret_err(test_name, timeStarted, "Wrong Answer")
    else:
        print('\033[0;32mPass\033[0m')
        return TestCase(test_name, "mediumend-test", time.time() - timeStarted, "", ""), True

if __name__ == '__main__':
    if len(sys.argv) != 5 and len(sys.argv) != 6:
        print("usage: python3 run_test.py <compiler> <ir_to_llvm.py> <libsysy_x86.a> <test_dir>")
        exit(1)
    compiler_path = sys.argv[1]
    converter_path = sys.argv[2]
    lib_path = sys.argv[3]
    test_dir = sys.argv[4]

    report_testcases = []

    testcases = get_testcases(test_dir)
    num_tests = len(testcases)
    pass_tests = 0
    for test_name in testcases:
        report_tc, ret = run_test(compiler_path, converter_path, lib_path, test_dir, test_name)
        if ret:
            pass_tests = pass_tests + 1
        report_testcases.append(report_tc)

    xml_name = "test_result.xml"
    if len(sys.argv) == 6:
        xml_name = sys.argv[5]
    with open(xml_name, "w") as f:
        TestSuite.to_file(f, [TestSuite(test_dir, report_testcases)])

    if pass_tests == num_tests:
        print(f'\033[0;32m{pass_tests}/{num_tests} tests Passed\033[0m')
    else:
        print(f'\033[0;31m{pass_tests}/{num_tests} tests Passed\033[0m')
        exit(1)

