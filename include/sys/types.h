/* ------------------------------------------------------------------------------------------------------
 * for system types
 * 2012 - 4 - 11
 * BY Casey 
 * -----------------------------------------------------------------------------------------------------*/

#ifndef __UNI_TYPES_H__
#define __UNI_TYPES_H__


typedef short           pid_t;
typedef unsigned long   size_t;
typedef unsigned long   off_t;
typedef unsigned long   time_t;

#define high(x)         (unsigned char)(((unsigned short)(x))>>8)
#define low(x)          (unsigned char)(x)
#define size(x)         (sizeof(x))
#define getw(ptr)       (*(unsigned short*)(ptr))
#define getb(ptr)       (*(unsigned char*)(ptr))
#define getdw(ptr)      (*(unsigned long*)(ptr))
#define getdd(ptr)      (*(unsigned long long*)(ptr))

#endif
