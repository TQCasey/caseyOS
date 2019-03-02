#include <stdarg.h>
#include <unistd.h>

int printf ( const char *fmt ,... )
{
    char buf[1024] = {0};
    va_list va_p = (void*)0;
    int len = 0;

    va_start (va_p,fmt);
    len =  vsprintf (buf,fmt,va_p);
    va_end (va_p);
    return (tty_write (buf,len));
}


int putchar ( char ch )
{
    char tmp[2] = {0};
    tmp[0] = ch;
    return (tty_write (tmp,1));
}

int puts ( const char *src )
{
    char *p = (char*)src;
    unsigned long len = strlen(p);
    int i = tty_write (src,len);
    tty_write ((char*)'\n',1);
    return (i+1);
}


