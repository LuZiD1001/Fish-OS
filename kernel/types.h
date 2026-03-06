/* ==============================================================================
   MyOS - Core Type Definitions (kernel/types.h)
   Freestanding environment has no standard library, so we define our own types
   ============================================================================== */

#pragma once

/* Fixed-width integer types */
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long long   int64_t;

typedef uint32_t           size_t;
typedef int32_t            ssize_t;
typedef uint32_t           uintptr_t;
typedef int32_t            intptr_t;

typedef uint8_t            bool;
#define true  1
#define false 0
#define NULL  ((void*)0)

/* Compiler hints */
#define PACKED    __attribute__((packed))
#define NORETURN  __attribute__((noreturn))
#define UNUSED    __attribute__((unused))
#define ALIGN(x)  __attribute__((aligned(x)))

/* Min/Max macros */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* Array size */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
