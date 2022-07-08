# 树莓派上的离线评测
# 需要先提供生成好的汇编代码压缩包

import os
import sys
import subprocess


TIMEOUT = 10

def unzip_packed_asm(zip_path):
    os.system(f'unzip {zip_path}')


def get_answer(path: str) -> tuple[list[str], int]:
    content = open(path).read().splitlines()
    return content[:-1], int(content[-1])


def judge_single(name, asm_dir, testcases_dir, temp_dir):
    source = os.path.join(asm_dir, f'{name}.S')
    executable = os.path.join(temp_dir, f'{name}')
    output = os.path.join(temp_dir, f'{name}.txt')

    input = os.path.join(testcases_dir, f'{name}.in')
    answer = os.path.join(testcases_dir, f'{name}.out')

    # do assembly and linking
    if os.system(f'gcc -march=armv7 {source} libsysy.a -o {executable}') != 0:
        print(f'\033[0;31m{name} Compile Error\033[0m')
        return False
        
    proc = subprocess.Popen(executable,
        stdin=open(input) if os.path.exists(input) else None,
        stdout=open(output, 'w')
    )
    try:
        proc.wait(TIMEOUT)
    except subprocess.TimeoutExpired:
        proc.kill()
        print(f'\033[0;31m{name} Time Limit Exceeded\033[0m')
        return False
        
    answer_content, answer_exitcode = get_answer(answer)
    if proc.returncode != answer_exitcode \
        or open(output).read().splitlines() != answer_content:
        print(f'\033[0;31m{name} Wrong Answer\033[0m')
        return False
        
    print(f'\033[0;32m{name} passed\033[0m')
    return True


def offline_judge(asm_dir, testcases_dir, temp_dir):
    srcs = [name for name in os.listdir(asm_dir) if name.endswith('.S')]
    failed_cases = []
    for src in sorted(srcs):
        name = src.replace('.S', '')
        ok = judge_single(name, asm_dir, testcases_dir, temp_dir)
        if not ok:
            failed_cases.append(name)
    
    failed = len(failed_cases)
    success = len(srcs) - failed
    print(f'judge: {success} success, {failed} failed')


if __name__ == '__main__':
    os.system('rm -r asm')
    unzip_packed_asm('asm.zip')
    offline_judge('asm', 'functional', 'temp')
