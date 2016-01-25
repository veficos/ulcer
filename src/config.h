

#ifndef _ULCER_CONFIG_H_
#define _ULCER_CONFIG_H_

#define DEBUG

/* stdlib */
#define USE_LIBNATIVE
#define USE_LIBSTR
#define USE_LIBHEAP
#define USE_LIBFILE
//#define USE_LIBSDL

#define ULCER_VERSION   "ulcer alpha 1.0.0"

#if defined(_WIN32) || defined(WIN32) 
#define _CRT_SECURE_NO_WARNINGS   1
#define _CRT_NONSTDC_NO_DEPRECATE 1
#define _CRTDBG_MAP_ALLOC 
#include <crtdbg.h>
#endif

#ifndef NULL
#   ifdef __cplusplus
#       define NULL    0
#else
#       define NULL    ((void *)0)
#   endif
#endif

#if defined(__STDC_VERSION__) || defined(__STDC__)
#   include <stdbool.h>
#   include <stdint.h>
#   include <limits.h>
#else

#ifndef bool
#   define bool           unsigned char
#endif

#ifndef false
#   define false          (0)
#endif

#ifndef true
#   define true           (!(false))
#endif

   typedef unsigned char        uint8_t;
   typedef char                 int8_t;
   typedef unsigned short       uint16_t;
   typedef short                int16_t;
   typedef unsigned int         uint32_t;
   typedef int                  int32_t;
   typedef unsigned long long   uint64_t;
   typedef long long            int64_t;
#endif

#endif