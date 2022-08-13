int main() {
  int a = 123456789;
  int a_ = a + 1;
  int _a = a - 1;
  int i = 0;
  while (i < 1000000) {
    int b = getint();
    putint(b / a);
    putch(32);
    putint(b % a);
    putch(32);
    putint(a * b);
    putch(32);
    putint(a_ * b);
    putch(32);
    putint(_a * b);
    putch(10);
    i = i + 1;
  }
  return 0;
}
