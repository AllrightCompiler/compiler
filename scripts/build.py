import os
import subprocess

TIMEOUT = 60

compiler = './compiler -O2'
testcases_dir = '/mnt/d/compiler-testcases/functional'
cross_gcc = 'arm-linux-gnueabihf-gcc'

def assembly(testcases, output_dir='asm'):
    # total = len(testcases)
    success, failed = 0, 0
    
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)
    for testcase in testcases:
        asm_name = testcase.replace('.sy', '.S')
        in_path = os.path.join(testcases_dir, testcase)
        out_path = os.path.join(output_dir, asm_name)

        command = f'{compiler} {in_path}'
        proc = subprocess.Popen(command, stdout=open(out_path, 'w'), shell=True)
        ok = True
        try:
            proc.wait(TIMEOUT)
        except subprocess.TimeoutExpired:
            proc.kill()
            print('\033[31m[TLE]\033[0m %s' % testcase)
            ok = False
        if proc.returncode:
            ok = False

        if ok:
            print('\033[32m[ok]\033[0m %s' % testcase)
            success += 1
        else:
            print('\033[31m[failed]\033[0m %s' % testcase)
            failed += 1
    print('[assembly] %d success, %d failed' % (success, failed))

    # zip assembly sources
    os.system('zip -r %s.zip %s' % (output_dir, output_dir))

if __name__ == '__main__':
    testcases = os.listdir(testcases_dir)
    testcases = [source for source in testcases if source.endswith('.sy')]

    assembly(testcases, 'asm')
