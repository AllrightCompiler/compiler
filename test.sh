#!/bin/sh

python test/main.py "$1" -t ../compiler2022/公开样例与运行时库/functional
python test/main.py "$1" -t ../compiler2021/公开用例与运行时库/2021初赛所有用例/functional
python test/main.py "$1" -t ../compiler2021/公开用例与运行时库/2021初赛所有用例/h_functional
python test/main.py "$1" -t ../compiler2021/公开用例与运行时库/function_test2021
python test/main.py "$1" -t ../compiler2021/公开用例与运行时库/function_test2020
python test/main.py "$1" -t ../compiler2022/公开样例与运行时库/performance -- -O2
python test/main.py "$1" -t ../compiler2021/公开用例与运行时库/2021初赛所有用例/performance -- -O2
python test/main.py "$1" -t ../compiler2021/公开用例与运行时库/performance_test2021-public -- -O2
python test/main.py "$1" -t ../compiler2021/公开用例与运行时库/performance_test2021-private -- -O2
