/*
 *  Copyright (c) 2020-2021 Thakee Nathees
 *  Distributed Under The MIT License
 */

#ifndef PK_COMMON_H
#define PK_COMMON_H

#include "include/pocketlang.h"

// Commonly used c standard headers across the sources. Don't include any
// headers that are specific to a single source here, instead include them in
// their source files explicitly (can not be implicitly included by another
// header). And don't include any C standard headers in any of the pocketlang
// headers.
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// __STDC_LIMIT_MACROS and __STDC_CONSTANT_MACROS are a workaround to
// allow C++ programs to use stdint.h macros specified in the C99
// standard that aren't in the C++ standard.
#define __STDC_LIMIT_MACROS
#include <stdint.h>

/*****************************************************************************/
/* INTERNAL CONFIGURATIONS                                                   */
/*****************************************************************************/

// Set this to dump compiled opcodes of each functions.
#define DEBUG_DUMP_COMPILED_CODE 0

// Set this to dump stack frame before executing the next instruction.
#define DEBUG_DUMP_CALL_STACK 0

// Nan-Tagging could be disable for debugging/portability purposes. See "var.h"
// header for more information on Nan-tagging.
#define VAR_NAN_TAGGING 1

// The maximum number of argument a pocketlang function supported to call. This
// value is arbitrary and feel free to change it. (Just used this limit for an
// internal buffer to store values before calling a new fiber).
#define MAX_ARGC 32

// The factor by which a buffer will grow when it's capacity reached.
#define GROW_FACTOR 2

// The initial minimum capacity of a buffer to allocate.
#define MIN_CAPACITY 8

/*****************************************************************************/
/* ALLOCATION MACROS                                                         */
/*****************************************************************************/

// Allocate object of [type] using the vmRealloc function.
#define ALLOCATE(vm, type) \
    ((type*)vmRealloc(vm, NULL, 0, sizeof(type)))

// Allocate object of [type] which has a dynamic tail array of type [tail_type]
// with [count] entries.
#define ALLOCATE_DYNAMIC(vm, type, count, tail_type) \
    ((type*)vmRealloc(vm, NULL, 0, sizeof(type) + sizeof(tail_type) * (count)))

// Allocate [count] amount of object of [type] array.
#define ALLOCATE_ARRAY(vm, type, count) \
    ((type*)vmRealloc(vm, NULL, 0, sizeof(type) * (count)))

// Deallocate a pointer allocated by vmRealloc before.
#define DEALLOCATE(vm, pointer) \
    vmRealloc(vm, pointer, 0, 0)

/*****************************************************************************/
/* COMMON MACROS                                                             */
/*****************************************************************************/

#if defined(__GNUC__)
  #pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined(__clang__)
  #pragma clang diagnostic ignored "-Wint-to-pointer-cast"
  #pragma clang diagnostic ignored "-Wunused-parameter"
#endif

#include <stdio.h> //< Only needed here for ASSERT() macro and for release mode
                   //< TODO; macro use this to print a crash report.

#define TOSTRING(x) #x
#define STRINGIFY(x) TOSTRING(x)

// CONCAT_LINE(X) will result evaluvated X<__LINE__>.
#define __CONCAT_LINE_INTERNAL_R(a, b) a ## b
#define __CONCAT_LINE_INTERNAL_F(a, b) __CONCAT_LINE_INTERNAL_R(a, b)
#define CONCAT_LINE(X) __CONCAT_LINE_INTERNAL_F(X, __LINE__)

// The internal assertion macro, this will print error and break regardless of
// the build target (debug or release). Use ASSERT() for debug assertion and
// use __ASSERT() for TODOs and assertions in public methods (to indicate that
// the host application did something wrong).
#define __ASSERT(condition, message)                                 \
  do {                                                               \
    if (!(condition)) {                                              \
      fprintf(stderr, "Assertion failed: %s\n\tat %s() (%s:%i)\n",   \
        message, __func__, __FILE__, __LINE__);                      \
      DEBUG_BREAK();                                                 \
      abort();                                                       \
    }                                                                \
  } while (false)

#define NO_OP do {} while (false)

#ifdef DEBUG

#ifdef _MSC_VER
  #define DEBUG_BREAK() __debugbreak()
#else
  #define DEBUG_BREAK() __builtin_trap()
#endif

// This will terminate the compilation if the [condition] is false, because of
// char _assertion_failure_<__LINE__>[-1] evaluated.
#define STATIC_ASSERT(condition) \
  static char CONCAT_LINE(_assertion_failure_)[2*!!(condition) - 1]

#define ASSERT(condition, message) __ASSERT(condition, message)

#define ASSERT_INDEX(index, size) \
  ASSERT(index >= 0 && index < size, "Index out of bounds.")

#define UNREACHABLE()                                                \
  do {                                                               \
    fprintf(stderr, "Execution reached an unreachable path\n"        \
      "\tat %s() (%s:%i)\n", __func__, __FILE__, __LINE__);          \
    DEBUG_BREAK();                                                   \
    abort();                                                         \
  } while (false)

#else

#define STATIC_ASSERT(condition) NO_OP

#define DEBUG_BREAK() NO_OP
#define ASSERT(condition, message) NO_OP
#define ASSERT_INDEX(index, size) NO_OP

// Reference : https://github.com/wren-lang/
#if defined(_MSC_VER)
  #define UNREACHABLE() __assume(0)
#elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))
  #define UNREACHABLE() __builtin_unreachable()
#elif defined(__EMSCRIPTEN__)
  #define UNREACHABLE() __builtin_unreachable()
#else
  #define UNREACHABLE() NO_OP
#endif

#endif // DEBUG

#if defined( _MSC_VER )
  #define forceinline __forceinline
#else
  #define forceinline __attribute__((always_inline))
#endif

// Using __ASSERT() for make it crash in release binary too.
#define TODO __ASSERT(false, "TODO: It hasn't implemented yet.")
#define OOPS "Oops a bug!! report please."

// The formated string to convert double to string. It'll be with the minimum
// length string representation of either a regular float or a scientific
// notation (at most 15 decimal points).
// Reference: https://www.cplusplus.com/reference/cstdio/printf/
#define DOUBLE_FMT "%.16g"

// Double number to string buffer size, used in sprintf with DOUBLE_FMT.
//  A largest number : "-1.234567890123456e+308"
// +  1 fot sign '+' or '-'
// + 16 fot significant digits
// +  1 for decimal point '.'
// +  1 for exponent char 'e'
// +  1 for sign of exponent
// +  3 for the exponent digits
// +  1 for null byte '\0'
#define STR_DBL_BUFF_SIZE 24

// Integer number to string buffer size, used in sprintf with format "%d".
// The minimum 32 bit integer = -2147483648
// +  1 for sign '-'
// + 10 for digits
// +  1 for null byte '\0'
#define STR_INT_BUFF_SIZE 12

// Integer number (double) to hex string buffer size.
// The maximum value an unsigned 64 bit integer can get is
// 0xffffffffffffffff which is 16 characters.
// + 16 for hex digits
// +  1 for sign '-'
// +  2 for '0x' prefix
// +  1 for null byte '\0'
#define STR_HEX_BUFF_SIZE 20

// Integer number (double) to bin string buffer size.
// The maximum value an unsigned 64 bit integer can get is
// 0b1111111111111111111111111111111111111111111111111111111111111111
// which is 64 characters.
// + 64 for bin digits
// +  1 for sign '-'
// +  2 for '0b' prefix
// +  1 for null byte '\0'
#define STR_BIN_BUFF_SIZE 68

#endif //PK_COMMON_H
