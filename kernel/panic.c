#include <stdarg.h>
#include <string.h>
#include <casey/kernel.h>
/* 
 * panic.c for kernel panic 
 * 2012 - 4 -15 By casey 
 */

/* ONLY KERNEL can do it */
int panic ( const char* fmt,... ) 
{
    va_list va_p = (void*)0;
    va_start(va_p,fmt);
    char buf[256 + 128] = {'\n'};

    ksetpt (0,0);
    ksetrgb (0x0c);
    vsprintf (buf+1,fmt,va_p);
    kputs (buf);

    va_end (va_p);

    __asm__ ("cli");
    while ( 1 ) ;
}


