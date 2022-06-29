#pragma once

#include <cstdint>
#include <cmath>

namespace armv7 {

/**
32位 armv7 有16个32位通用寄存器（其实pc不算）
这里假设处理器同时支持Thumb-2和vfp的某个版本

vfp的32位浮点寄存器应该是有32个，s0~s15是caller saved, s16~s31是callee saved
这组寄存器还可以作为双精度的d0~d15，以及quadword的q0~q7
abi上应该要求是hardfp，前8个浮点参数通过寄存器传递
*/

constexpr int r0 = 0;
constexpr int r1 = 1;
constexpr int r2 = 2;
constexpr int r3 = 3;
constexpr int r4 = 4;
constexpr int r5 = 5;
constexpr int r6 = 6;
constexpr int r7 = 7;
constexpr int r8 = 8;
constexpr int r9 = 9;
constexpr int r10 = 10;
constexpr int r11 = 11;
constexpr int r12 = 12;
constexpr int r13 = 13;
constexpr int r14 = 14;
constexpr int r15 = 15;
constexpr int ip = r12;
constexpr int sp = r13;
constexpr int lr = r14;
constexpr int pc = r15;

constexpr int s0 = 0;

constexpr int GPR_SIZE = 4;

constexpr int NR_GPRS = 16; // 通用寄存器数量
constexpr int NR_FPRS = 32; // 浮点寄存器数量

enum RegisterAttr {
  Special,
  Volatile, // caller saved
  NonVolatile, // callee saved
};

constexpr RegisterAttr GPRS_ATTR[NR_GPRS] = {
  Volatile, Volatile, Volatile, Volatile, // r0 ~ r3
  NonVolatile, NonVolatile, NonVolatile, NonVolatile, NonVolatile, // r4 ~ r8
  NonVolatile, // r9
  NonVolatile, NonVolatile, // r10, r11
  Volatile, Special, NonVolatile, Special, // ip, sp, lr, pc
};

// TODO: 确认vfp寄存器属性

constexpr bool gpr_allocable(int reg) {
  return GPRS_ATTR[reg] == Volatile || GPRS_ATTR[reg] == NonVolatile;
}

constexpr bool fpr_allocable(int reg) {
  return 0 <= reg && reg < 32;
}

constexpr int NR_ARG_GPRS = 4;
constexpr int ARG_GPRS[NR_ARG_GPRS] = {r0, r1, r2, r3};

constexpr int NR_ARG_FPRS = 8;

// <imm8m>: 无符号8位立即数循环右移偶数位得到
inline bool is_imm8m(int x) {
  uint32_t v = uint32_t(x);
  for (int i = 0; i < 32; i += 2) {
    uint32_t t = (v << i) | (v >> (32 - i)); // 循环左移回去
    if (t <= 0xff)
      return true; 
  }
  return false;
}

// Thumb-2指令集使用的12位立即数
inline bool is_imm12(int x) {
  uint32_t v = uint32_t(x);
  return 0 <= v && v < 4096;
}

// vmov.f32使用的立即数
inline bool is_vmov_f32_imm(float x) {
  float a = std::abs(x);
  for (int n = 16; n <= 31; ++n)
    for (int r = 0; r <= 7; ++r)
      if (a == std::ldexp(float(n), -r))
        return true;
  return false;
}

// load和store的偏移量
inline bool is_valid_ldst_offset(int x) {
  return -4095 <= x && x <= 4095;
}

} // namespace armv7
