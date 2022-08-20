from random import randint

if __name__ == '__main__':
    xs = [-(1 << 31), 0xffff_ffff, 0, 1, 2, (1 << 31) - 1, 3, -1, -2, -3]
    xs.extend([randint(-(1 << 31), 1 << 31) for _ in range(len(xs), 1_000_000)])
    for x in xs:
        print(x)
