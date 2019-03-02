#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

void spin ( const char *fmt,... ) 
{
    va_list va_p = (void*)0;
    va_start(va_p,fmt);
    char buf[256 + 128] = {0};

    vsprintf (buf,fmt,va_p);
    printf (buf);

    va_end (va_p);

    unsigned long pid = 0,tasks_nr = 0;
    pid = (get_pinfo (&tasks_nr))>>16;

    if ( pid < tasks_nr )
    {
        __asm__ ("cli");     /* this will be ok in user proc 
                              * just cause a #GP 
                              */
    }
    while ( 1 );
}

void assert_fail (const char* expression,
                  const char* local_file,
                  const char* base_file,
                  unsigned long line_nr ) 
{
    spin ("assert( %s ) failed!!!\nlocal file: %s,base_file: %s,line: %u",
            expression,local_file,base_file,line_nr )  ;

    
    /*should never arrive here!!!!!*/
    __asm__ ("ud2");
}

