import os
import subprocess
import sys

from argparse import ArgumentParser
from enum import Enum, auto
from glob import glob
from tempfile import TemporaryDirectory
from typing import NamedTuple, Optional, Union

from parse import parse


TEST_ROUND = 5
TIMEOUT = 120


class Config(NamedTuple):
    compiler: str
    testcases: str
    compiler_args: str


class Result(Enum):
    LINKER_ERROR = auto()
    PASSED = auto()
    WRONG_ANSWER = auto()
    TIME_LIMIT_EXCEEDED = auto()
    GCC_ERROR = auto()


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


def run(
    workdir: str,
    assemble: str,
    input: str,
    answer: str,
    round: int
) -> Union[Result, float]:
    executable = os.path.join(workdir, 'main')
    output = os.path.join(workdir, 'output')
    time = os.path.join(workdir, 'time')
    if os.system(f'gcc -march=armv7 {assemble} runtime/libsysy.a'
                 f' -o {executable}') != 0:
        return Result.LINKER_ERROR
    answer_content, answer_exitcode = get_answer(answer)
    average_time = 0
    timing = True
    for _ in range(round):
        proc = subprocess.Popen(
            executable,
            stdin=open(input) if os.path.exists(input) else None,
            stdout=open(output, 'w'), stderr=open(time, 'w'))
        try:
            proc.wait(TIMEOUT)
        except subprocess.TimeoutExpired:
            proc.kill()
            return Result.TIME_LIMIT_EXCEEDED
        if proc.returncode != answer_exitcode \
                or open(output).read().splitlines() != answer_content:
            return Result.WRONG_ANSWER
        if round > 1:
            print('.', end='', flush=True)
        t = get_time(time)
        if t is None:
            timing = False
        else:
            average_time += t
    if timing:
        return average_time / 1_000 / TEST_ROUND
    else:
        return Result.PASSED


def test(config: Config, testcase: str) -> bool:
    with TemporaryDirectory() as tempdir:
        print(testcase, end=': ', flush=True)
        source = os.path.join(config.testcases, f'{testcase}.sy')
        input = os.path.join(config.testcases, f'{testcase}.in')
        answer = os.path.join(config.testcases, f'{testcase}.out')
        assemble = os.path.join(tempdir, 'asm.s')
        command = (f'{config.compiler} {source}'
                   f' -o {assemble} {config.compiler_args}')
        proc = subprocess.Popen(command, shell=True)
        try:
            proc.wait(TIMEOUT)
        except subprocess.TimeoutExpired:
            proc.kill()
            print('\033[0;31mCompiler TLE\033[0m')
            return False
        if proc.returncode != 0:
            print('\033[0;31mCompiler Error\033[0m')
            return False
        result = run(tempdir, assemble, input, answer, TEST_ROUND)
        if result == Result.LINKER_ERROR:
            print('\033[0;31mLinker Error\033[0m')
            return False
        elif result == Result.WRONG_ANSWER:
            print('\033[0;31mWrong Answer\033[0m')
            return False
        elif result == Result.TIME_LIMIT_EXCEEDED:
            print('\033[0;31mTime Limit Exceeded\033[0m')
            return False
        else:
            runtime = result
        print(' ', end='')
        if not isinstance(runtime, float) or runtime == 0:
            print('\033[0;32mPassed\033[0m')
            return True
        result = Result.GCC_ERROR \
            if os.system(
                'gcc -march=armv7 -xc++ -O2 -S'
                f' -include runtime/sylib.h {source} -o {assemble}') != 0 \
            else run(tempdir, assemble, input, answer, 1)
        if isinstance(result, Result):
            print('\033[0;31mGCC Error\033[0m')
        else:
            print(f'\033[0;32m{runtime :.3f}ms / {result :.3f}ms'
                  f' = {result / runtime :.2%}\033[0m')
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
