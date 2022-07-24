import os
import sys
import time
import subprocess
import multiprocessing
from junit_xml import TestSuite, TestCase
from multiprocessing import Pool
from parse import parse

TIMEOUT = 120

def get_testcases(test_dir):
    testcases = []
    for file in os.listdir(test_dir):
        if os.path.splitext(file)[1] == ".sy":
            testcases.append(os.path.splitext(file)[0])
    testcases.sort()
    return testcases

def ret_err(test_name, timeStarted, msg):
    tc = TestCase(test_name, "mediumend-test", time.time() - timeStarted)
    tc.add_failure_info(msg)
    return tc, False

def get_time(err_file):
    content = open(err_file).read().splitlines()
    if not content:
        return -1
    content = content[-1]
    h, m, s, us = map(int, parse('TOTAL: {}H-{}M-{}S-{}us', content))
    return ((h * 60 + m) * 60 + s) + us / 1_000_000

def get_runtime(args, compiler, opt_flag):
    lib_path, test_dir, test_name = args[2], args[3], args[4]

    lib_h_path = lib_path[0 : lib_path.rfind("/") + 1] + "sylib.h"
    lib_c_path = lib_path[0 : lib_path.rfind("/") + 1] + "sylib.c"
    code_path = os.path.join(test_dir, test_name + ".sy")
    new_code_path = os.path.join(test_dir, test_name + ".cpp")
    with open(new_code_path, "w") as f:
        f.write(open(lib_h_path, "r").read())
        f.write("\n")
    with open(new_code_path, "a") as f:
        lines = open(lib_c_path, "r").readlines()
        for line in lines:
            if line.strip() != '#include"sylib.h"':
                f.write(line + "\n")
    with open(new_code_path, "a") as f:
        f.write(open(code_path, "r").read())
        f.write("\n")
    new_name = test_name + f"_{compiler}{opt_flag}"

    input_path = os.path.join(test_dir, test_name + ".in")
    output_path = os.path.join(test_dir, new_name + ".tmp")
    outerr_path = os.path.join(test_dir, new_name + ".err")
    exe_path = os.path.join(test_dir, new_name)

    command = f'{compiler} {opt_flag} {new_code_path} -o {exe_path}'
    proc = subprocess.Popen(command, stdout=open("/dev/null", "w"), stderr=open("/dev/null", "w"), shell=True)
    proc.wait()
    if proc.returncode != 0:
        return TIMEOUT

    command = f'{exe_path}'
    timeStarted = time.time()
    if (os.path.exists(input_path)):
        proc = subprocess.Popen(command, stdin=open(input_path, "r"), stdout=open(output_path, "w",), stderr=open(outerr_path, "w"), shell=True)
    else:
        proc = subprocess.Popen(command, stdout=open(output_path, "w"), stderr=open(outerr_path, "w"), shell=True)
    proc.wait()
    
    runTime = time.time() - timeStarted
    t_err = get_time(outerr_path)
    if t_err > 0:
        return t_err
    return runTime

def run_test(args):
    compiler_path, converter_path, lib_path, test_dir, test_name = args[0], args[1], args[2], args[3], args[4]
    code_path = os.path.join(test_dir, test_name + ".sy")
    input_path = os.path.join(test_dir, test_name + ".in")
    output_path = os.path.join(test_dir, test_name + ".tmp")
    outerr_path = os.path.join(test_dir, test_name + ".err")
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
        proc = subprocess.Popen(command, stdin=open(input_path, "r"), stdout=open(output_path, "w",), stderr=open(outerr_path, "w"), shell=True)
    else:
        proc = subprocess.Popen(command, stdout=open(output_path, "w"), stderr=open(outerr_path, "w"), shell=True)
    try:
        proc.wait(TIMEOUT)
        runTime = time.time() - timeStarted
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
        t_err = get_time(outerr_path)
        if t_err > 0: # Performance
            runTime = t_err
        # gcc_time = get_runtime(args, "gcc", "")
        gcc_O2_time = get_runtime(args, "gcc", "-O2")
        # clang_time = get_runtime(args, "clang", "")
        clang_O2_time = get_runtime(args, "clang", "-O2")
        minTime = min(gcc_O2_time, clang_O2_time)
        # minTime = min(min(gcc_time, gcc_O2_time), min(clang_time, clang_O2_time))
        log_str = f"s: {(minTime / runTime):.6f}\ngcc-O2: {gcc_O2_time:.6f}\nclang-O2: {clang_O2_time:.6f}"
        # log_str = f"time: {runTime}\ntime-opt: {minTime}\n\ngcc: {gcc_time}\ngcc-O2: {gcc_O2_time}\nclang: {clang_time}\nclang-O2: {clang_O2_time}"
        return TestCase(test_name, "mediumend-test", elapsed_sec=runTime, file=log_str), True

if __name__ == '__main__':
    if len(sys.argv) != 5 and len(sys.argv) != 6:
        print("usage: python3 run_test.py <compiler> <ir_to_llvm.py> <libsysy_x86.a> <test_dir>")
        exit(1)
    compiler_path = sys.argv[1]
    converter_path = sys.argv[2]
    lib_path = sys.argv[3]
    test_dir = sys.argv[4]

    testcases = get_testcases(test_dir)

    # Parallel
    parallel_args = []
    for test_name in testcases:
        parallel_args.append((compiler_path, converter_path, lib_path, test_dir, test_name))
    with Pool(multiprocessing.cpu_count()) as p:
        ret = p.map(run_test, parallel_args)

    num_tests = len(testcases)
    pass_tests = 0
    report_testcases = []
    for pair in ret:
        if pair[1]:
            pass_tests = pass_tests + 1
        report_testcases.append(pair[0])

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

