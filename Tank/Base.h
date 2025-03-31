#pragma once

/// \file
/// \brief This file contains the most basic utilities.
// Rand函数的作用是随机生成一个整数，范围在[0,n)之间，用法是Rand(n).
// Rand01函数的作用是随机生成一个double类型的数，范围在[0,1)之间，用法是Rand01().
// vec结构体是一个二维向量，包含两个整数x和y，提供了一些常用的操作函数，如Eq, Neq, Add, Sub, Mul等.
// Eq函数用于判断两个Vec是否相等，Neq函数用于判断两个Vec是否不相等.
// Add函数用于将两个Vec相加，Sub函数用于将两个Vec相减，Mul函数用于将Vec与一个整数相乘.
// RandVec函数的作用是随机生成一个Vec类型的数，范围在[0,v.x) * [0,v.y)，用法是RandVec(v).
// SleepMs函数的作用是让当前线程休眠指定的毫秒数，参数time为毫秒数.
// Daze函数的作用是让当前线程暂停执行，具体实现根据编译器和平台不同而不同.
#if defined(__MINGW64_VERSION_MAJOR) && __MINGW64_VERSION_MAJOR < 7
  #error Your MinGW version is too old. Please update it to a newer version.
#endif

#include "detail/Macros.h"

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#if _WIN32
  #include <Synchapi.h>
#else
  #include <unistd.h>
#endif

/// \brief An alias of `unsigned int`.
typedef unsigned uint;

typedef int8_t int8;
typedef uint8_t uint8;

typedef int64_t int64;
typedef uint64_t uint64;

/// \brief Randomly generate an `int` in `[0, n)`.
// 这里我修改了此随机函数，使得它可以生成min到max之间的任何整数
int Rand(int min, int max) {
  TK_ASSERT(max <= RAND_MAX,
            "`n` is too large to be handled by `Rand`, please implement your own random number generator");
  int temp = 0;
  while (temp < min) {
    temp = rand() % max;
  }
  return temp;
}

/// \brief Randomly generate a `double` in `[0, 1)`.
double Rand01(void) {
  return (double)rand() / ((double)RAND_MAX + 1.0);
}

//
//
//
/// \brief A 2D vector which contains only `int`s.
typedef struct {
  int x, y;
} Vec;

bool Eq(Vec a, Vec b) {
  return a.x == b.x && a.y == b.y;
}

bool Neq(Vec a, Vec b) {
  return !Eq(a, b);
}

Vec Add(Vec a, Vec b) {
  return (Vec){a.x + b.x, a.y + b.y};
}

Vec Sub(Vec a, Vec b) {
  return (Vec){a.x - b.x, a.y - b.y};
}

Vec Mul(Vec a, int b) {
  return (Vec){a.x * b, a.y * b};
}

/// \brief Randomly generate a `Vec` in `[0, v.x) * [0, v.y)`.
Vec RandVec(Vec v) {
  TK_ASSERT(v.x <= RAND_MAX && v.y <= RAND_MAX,
            "`v` is too large to be handled by `RandVec`, please implement your own random number generator");
  return (Vec){rand() % v.x, rand() % v.y};
}

//
//
//
typedef const char *Color;
#define TK_RUNTIME_COLOR "%s"

#define TK_NORMAL        "0"
#define TK_BLACK         "30"
#define TK_RED           "31"
#define TK_GREEN         "32"
#define TK_YELLOW        "33"
#define TK_BLUE          "34"
#define TK_MAGENTA       "35"
#define TK_CYAN          "36"
#define TK_WHITE         "37"
#define TK_BRIGHT_YELLOW "93"
#define TK_BRIGHT_BLUE   "94"

#define __TK_SET_COLOR(color) "\033[" color "m"

#define __TK_TEXT1(text)        text
#define __TK_TEXT2(text, color) __TK_SET_COLOR(color) text __TK_SET_COLOR(TK_NORMAL)

/// \brief Get an optionally colored text.
///
/// \example ```c
/// // Colors can be specified at compile-time, by using `TK_RED`, etc.
/// printf(TK_TEXT("Hello World!\n"));
/// printf(TK_TEXT("Hello World!\n", TK_RED));
/// printf(TK_TEXT("Hello World%c\n", TK_YELLOW), '!');
/// printf(TK_TEXT("Hello World%s\n", TK_GREEN), "!");
///
/// // Colors can also be specified at runtime, by using `Color` and `TK_RUNTIME_COLOR`.
/// Color color = TK_BLUE;
/// printf(TK_TEXT("Hello World%c\n", TK_RUNTIME_COLOR), color, '!');
/// ```
#define TK_TEXT(...) __TK_EXPAND(__TK_EXPAND(TK_CONCAT(__TK_TEXT, TK_NUM_OF(__VA_ARGS__)))(__VA_ARGS__))

//
//
//
/// \brief Suspends the execution of the current thread until
/// the time-out interval (in milliseconds) elapses.
///
/// \example ```c
/// SleepMs(1000); // Sleep for 1000 milliseconds.
/// ```
void SleepMs(uint time) {
#ifdef _WIN32
  Sleep(time);
#else
  usleep(time * 1000);
#endif // _WIN32
}

/// \brief Suspends the execution of the current thread for
/// only several nanoseconds.
void Daze(void) {
#if TK_ICC || TK_MSVC
  _mm_pause();
#elif __arm__ || __aarch64__
  __builtin_arm_yield();
#else
  __builtin_ia32_pause();
#endif
}
