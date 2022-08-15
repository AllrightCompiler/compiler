build/debug/compiler -O2 test/mul_div_mod_const/test.c -o temp.s
gcc temp.s runtime/libsysy.a -o temp
./temp < temp.in > temp.out
gcc -include runtime/sylib.h -O2 test/mul_div_mod_const/test.c runtime/libsysy.a -o temp
./temp < temp.in > std.out
diff temp.out std.out
