#ifndef _NT_DEFINES_H_
#define _NT_DEFINES_H_

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <err.h>

#if defined(__GNUC__)
  #define NT_STATIC_INLINE        static __inline__ __attribute__((always_inline))
  #define NT_STATIC_PURE_INLINE   static __inline__ __attribute__((always_inline, pure))
  #define NT_EXPECT(cond, expect) __builtin_expect(cond, expect)
  #define NT_ATTR(expr)      __attribute__(expr)
  /*((format (printf, 2, 3)));*/
  #if (__GNUC__ >= 4) && !defined(__objc__)
    /* Not yet implemented for obj-c in GCC */
    #define NT_HAVE_CONSTRUCTOR   1
    #define NT_HAVE_DESTRUCTOR    1
  #endif
#else
  #define NT_STATIC_INLINE        static __inline__
  #define NT_STATIC_PURE_INLINE   static __inline__
  #define NT_EXPECT(cond, expect) cond
  #define NT_ATTR(expr)
#endif

/**
  The constructor attribute causes the function to be called automatically
  before execution enters main().
*/
#define NT_CONSTRUCTOR  NT_ATTR((constructor))

/**
  The destructor attribute causes the function to be called automatically
  after main() has completed or exit() has been called.
*/
#define NT_DESTRUCTOR  NT_ATTR((destructor))


/* Assert zero return value */
#define AZ(foo)	do { assert((foo) == 0); } while(0)
#define AN(foo)	do { assert((foo) != 0); } while(0)

/* Align a value */
#define NT_ALIGN(value, size) (((value)+(size)-1)&~((size)-1))
/* Align to memory, counted in bytes */
#if __LP64__
  #define NT_ALIGN_M(value) NT_ALIGN(value, 8)
#else
  #define NT_ALIGN_M(value) NT_ALIGN(value, 4)
#endif

/* Filename macro */
#ifndef __FILENAME__
  #define __FILENAME__ ((strrchr(__FILE__, '/') ?: __FILE__ - 1) + 1)
#endif

/* Source module identifier */
#ifdef SRC_MODULE
  #undef SRC_MODULE
#endif
#define SRC_MODULE __FILENAME__

/* Macros enabled if NT_DEBUG */
#if NT_DEBUG
  #define log_debug(fmt, ...) fprintf(stderr, "%s [%d] D %s:%d: " fmt "\n",\
    SRC_MODULE, getpid(), __FILE__, __LINE__, ##__VA_ARGS__)
#else
  #define log_debug(fmt, ...) ((void)0)
#endif

/* Macros enabled if NT_LOG_TRACE */
#if NT_LOG_TRACE
  #define log_trace(fmt, ...) fprintf(stderr, "%s [%d] T %s:%d in %s " fmt "\n", \
    SRC_MODULE, getpid(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
  #define log_trace(fmt, ...) ((void)0)
#endif

#if NT_DEBUG || NT_LOG_TRACE
  #include <unistd.h>
  #include <stdio.h>
#endif

/* Some generic types */
typedef struct nt_tuple_t {
  void *a;
  void *b;
} nt_tuple_t;

typedef uint8_t byte_t;

#endif /* _NT_DEFINES_H_ */
