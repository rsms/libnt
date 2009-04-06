#ifndef _NT_DEFINES_H_
#define _NT_DEFINES_H_

#if defined(__GNUC__) && (__GNUC__ >= 4)
  #define NT_STATIC_INLINE        static __inline__ __attribute__((always_inline))
  #define NT_STATIC_PURE_INLINE   static __inline__ __attribute__((always_inline, pure))
  #define NT_EXPECT(cond, expect) __builtin_expect(cond, expect)
  #define NT_ATTR(attr, ...)      __attribute__((attr, ##__VA_ARGS__))
  #define NT_HAVE_CONSTRUCTOR     1
#else
  #define NT_STATIC_INLINE        static __inline__
  #define NT_STATIC_PURE_INLINE   static __inline__
  #define NT_EXPECT(cond, expect) cond
  #define NT_ATTR(attr, ...)
#endif /* gcc >=4.0 */

#define NT_CONSTRUCTOR  NT_ATTR(constructor)

#ifndef __FILENAME__
  #include <string.h>
  #define __FILENAME__ ((strrchr(__FILE__, '/') ?: __FILE__ - 1) + 1)
#endif

#ifdef SRC_MODULE
  #undef SRC_MODULE
#endif
#define SRC_MODULE __FILENAME__

#if DEBUG
  #define log_debug(fmt, ...) fprintf(stderr, "%s [%d] D %s:%d: " fmt "\n",\
    SRC_MODULE, getpid(), __FILE__, __LINE__, ##__VA_ARGS__)
#else
  #define log_debug(fmt, ...) ((void)0)
#endif

#if NT_LOG_TRACE
  #define log_trace(fmt, ...) fprintf(stderr, "%s [%d] T %s:%d in %s " fmt "\n", \
    SRC_MODULE, getpid(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
  #define log_trace(fmt, ...) ((void)0)
#endif

#if DEBUG || NT_LOG_TRACE
  #include <unistd.h>
  #include <stdio.h>
#endif


#endif /* _NT_DEFINES_H_ */
