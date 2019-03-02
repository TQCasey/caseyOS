#include <casey/types.h>
#include <stdarg.h>
#include <string.h>
#include <casey/kernel.h>

extern int  kputch ( const char ch ) ;

int  kputs ( const char* str ) 
{
    char *p = (char*)str;
    unsigned long cnt = 0;
    while ( *p ) {
        cnt ++;
        kputch (*p++);
    }
    return (cnt);
}

int printk ( const char* fmt ,... ) 
{
    int len = 0 ;
    char buf[1024] = {0};

    va_list va_p = NULL;

    va_start (va_p,fmt);
    len = vsprintf (buf,fmt,va_p);
    len = kputs (buf);
    va_end (va_p);
    return ( len );
}
