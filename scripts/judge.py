# 树莓派上的离线评测
# 需要先提供生成好的汇编代码压缩包

import os
import re
import sys
import subprocess


TIMEOUT = 60

def unzip_packed_asm(zip_path):
    os.system(f'unzip {zip_path}')


def get_normalized_content(path: str) -> list[str]:
    content = open(path).read().splitlines()
    content = list(map(lambda s: s.strip(), content))
    return content


def get_answer(path: str) -> tuple[list[str], int]:
    content = get_normalized_content(path)
    return content[:-1], int(content[-1])


def parse_time(result_path):
    try:
        with open(result_path, 'r') as f:
            lines = f.readlines()
            r = re.findall('(\d+)H-(\d+)M-(\d+)S-(\d+)us', lines[-1])
            h, m, s, us = map(int, r[0])
            total_us = ((h * 60 + m) * 60 + s) * 1000000 + us
            total_ms = total_us / 1000
            return total_ms
    except:
        return 0


def judge_single(name, asm_dir, testcases_dir, temp_dir):
    source = os.path.join(asm_dir, f'{name}.S')
    executable = os.path.join(temp_dir, f'{name}')
    output = os.path.join(temp_dir, f'{name}.txt')
    stderr = os.path.join(temp_dir, f'{name}.stderr')

    input = os.path.join(testcases_dir, f'{name}.in')
    answer = os.path.join(testcases_dir, f'{name}.out')

    # do assembly and linking
    if os.system(f'gcc -march=armv7 {source} libsysy.a -o {executable}') != 0:
        print(f'\033[0;31m{name} Compile Error\033[0m')
        return False, 0
        
    proc = subprocess.Popen(executable,
        stdin=open(input) if os.path.exists(input) else None,
        stdout=open(output, 'w'),
        stderr=open(stderr, 'w'),
    )
    try:
        proc.wait(TIMEOUT)
    except subprocess.TimeoutExpired:
        proc.kill()
        print(f'\033[0;31m{name} Time Limit Exceeded\033[0m')
        return False, 0
        
    answer_content, answer_exitcode = get_answer(answer)
    if proc.returncode != answer_exitcode \
        or get_normalized_content(output) != answer_content:
        print(f'\033[0;31m{name} Wrong Answer\033[0m')
        return False, 0

    time = parse_time(stderr)
    print(f'\033[0;32m{name} passed\033[0m')
    return True, time


def offline_judge(asm_dir, testcases_dir, temp_dir):
    srcs = [name for name in os.listdir(asm_dir) if name.endswith('.S')]
    failed_cases = []
    results = ['name,ok,time']
    for src in sorted(srcs):
        name = src.replace('.S', '')
        try:
            ok, time = judge_single(name, asm_dir, testcases_dir, temp_dir)
        except KeyboardInterrupt:
            print(f'\033[0;33m{name} interrupted\033[0m')
            ok = False
        if not ok:
            failed_cases.append(name)
        results.append(f'{name},{ok},{time}')
    
    failed = len(failed_cases)
    success = len(srcs) - failed
    print(f'judge: {success} success, {failed} failed')

    with open('results.csv', 'w') as f:
        f.write('\n'.join(results))


if __name__ == '__main__':
    os.system('rm -r asm')
    unzip_packed_asm('asm.zip')

    if len(sys.argv) > 1:
        offline_judge('asm', 'performance', 'temp')
    else:
        offline_judge('asm', 'functional', 'temp')
