/* ---------------------------------------------------------------------------
 * this data types just made specially for casey os ^_^
 * ctypes.h 
 * 2012 - 03 -05 
 * By Casey
 * --------------------------------------------------------------------------*/

#ifndef __TYPE_H__
#define __TYPE_H__

//types
typedef unsigned char         __u8  ;
typedef unsigned short        __u16 ;
typedef unsigned long         __u32 ;
typedef unsigned long long    __u64 ;
typedef char                  __i8  ;
typedef short                 __i16 ;
typedef long                  __i32 ;
typedef long long             __i64 ;
typedef unsigned char         __byte;
typedef unsigned char          bool;
typedef void*                  PRC;


typedef __u32                   u32;
typedef __u16                   u16;
typedef __u8                    u8;
typedef __i32                   i32;
typedef __i16                   i16;
typedef __i8                    i8;

//macro
#define FALSE 		    0
#define TRUE 		    1

#define true            1
#define false           0

#ifndef NULL
#define NULL 		    ((void*)0)
#endif

#define SIZE(x)         (sizeof(x))
#define NUM(x)          (sizeof(x)/sizeof(x[0]))

#endif
