import os
import subprocess
import sys

from argparse import ArgumentParser
from glob import glob
from tempfile import TemporaryDirectory
from typing import NamedTuple, Optional

from parse import parse


TEST_ROUND = 1
TIMEOUT = 10


class Config(NamedTuple):
    compiler: str
    testcases: str
    compiler_args: str


def get_config(argv: list[str]) -> Config:
    parser = ArgumentParser('simple-tester')
    parser.add_argument('compiler', help='path to the compiler')
    parser.add_argument('-t', '--testcases',
                        metavar='<testcases>', required=True,
                        help='path to the directory containing testcases')
    index: int
    try:
        index = argv.index('--')
    except ValueError:
        index = len(argv)
    args = parser.parse_args(argv[:index])
    return Config(compiler=args.compiler,
                  testcases=args.testcases,
                  compiler_args=' '.join(argv[index + 1:]))


def get_testcases(config: Config) -> list[str]:
    testcases = [os.path.splitext(os.path.basename(file))[0]
                 for file in glob(os.path.join(config.testcases, '*.sy'))]
    testcases.sort()
    return testcases


def get_answer(file: str) -> tuple[list[str], int]:
    content = open(file).read().splitlines()
    return content[:-1], int(content[-1])


def get_time(file: str) -> Optional[int]:
    content = open(file).read().splitlines()
    if not content:
        return None
    content = content[-1]
    h, m, s, us = map(int, parse('TOTAL: {}H-{}M-{}S-{}us', content))
    return ((h * 60 + m) * 60 + s) * 1_000_000 + us


def test(config: Config, testcase: str) -> bool:
    with TemporaryDirectory() as tempdir:
        print(testcase, end=': ', flush=True)
        source = os.path.join(config.testcases, f'{testcase}.sy')
        input = os.path.join(config.testcases, f'{testcase}.in')
        answer = os.path.join(config.testcases, f'{testcase}.out')
        assemble = os.path.join(tempdir, 'asm.s')
        executable = os.path.join(tempdir, 'main')
        output = os.path.join(tempdir, 'output')
        time = os.path.join(tempdir, 'time')
        command = f'{config.compiler} {source} \
                -o {assemble} {config.compiler_args}'
        proc = subprocess.Popen(command, shell=True)
        try:
            proc.wait(TIMEOUT)
        except subprocess.TimeoutExpired:
            proc.kill()
            print('\033[0;31mCompiler TLE\033[0m')
            return False
        if proc.returncode != 0 \
                or os.system(f'gcc -march=armv7 {assemble} runtime/libsysy.a \
                    -o {executable}') != 0:
            print('\033[0;31mCompiler Error\033[0m')
            return False
        answer_content, answer_exitcode = get_answer(answer)
        average_time = 0
        timing = True
        for i in range(TEST_ROUND):
            proc = subprocess.Popen(
                executable,
                stdin=open(input) if os.path.exists(input) else None,
                stdout=open(output, 'w'), stderr=open(time, 'w'))
            try:
                proc.wait(TIMEOUT)
            except subprocess.TimeoutExpired:
                proc.kill()
                print('\033[0;31mTime Limit Exceeded\033[0m')
                return False
            if proc.returncode != answer_exitcode \
                    or open(output).read().splitlines() != answer_content:
                print('\033[0;31mWrong Answer\033[0m')
                return False
            print('.', end='', flush=True)
            t = get_time(time)
            if t is None:
                timing = False
            else:
                average_time += t
        message = f'{round(average_time / TEST_ROUND, 4):.3f}' \
            if timing else 'Passed'
        print(f' \033[0;32m{message}\033[0m')
        return True


if __name__ == '__main__':
    config = get_config(sys.argv[1:])
    testcases = get_testcases(config)
    failed = []
    for testcase in testcases:
        if not test(config, testcase):
            failed.append(testcase)
    info = '\033[0;34m[info]\033[0m {}'
    if not failed:
        print(info.format('All Passed'))
    else:
        for testcase in failed:
            print(info.format(f'`{testcase}` Failed'))
