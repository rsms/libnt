#ifndef _NT_DEFINES_H_
#define _NT_DEFINES_H_

#if defined (__GNUC__) && (__GNUC__ >= 4)
#define NT_STATIC_INLINE        static __inline__ __attribute__((always_inline))
#define NT_STATIC_PURE_INLINE   static __inline__ __attribute__((always_inline, pure))
#define NT_EXPECT(cond, expect) __builtin_expect(cond, expect)
#define NT_ATTR(attr, ...)      __attribute__((attr, ##__VA_ARGS__))
#else
#define NT_STATIC_INLINE        static __inline__
#define NT_STATIC_PURE_INLINE   static __inline__
#define NT_EXPECT(cond, expect) cond
#define NT_ATTR(attr, ...)
#endif

#endif /* _NT_DEFINES_H_ */
