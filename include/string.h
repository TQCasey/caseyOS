/*--------------------------------------------------------
* For String Operations
* 2011-12-10
* By Casey
*--------------------------------------------------------*/
#ifndef __STRING_H__
#define __STRING_H__

#include <stdarg.h>

#define NULL_BK		0x0F
#define Max(x,y)  ((x)>(y) ? (x):(y))
#define Min(x,y)  ((x)<(y) ? (x):(y))



/* 
 * kernel space can use this funcs also ,put them in header files 
 * to share code with kernel ,we can do it !!!!!!!!!!!!!!!
 */

//prototyps
extern int    strlen ( const char *str );
extern char*  strcat ( char* dest,const char *src );
extern char*  strcpy ( char *dest,const char *src );
extern int    strcmp ( const char *str1,const char *str2 );
extern int    strncmp ( const char *str1,const char *str2,int len  ); 
extern char*  strncpy ( char *dest,const char *src,int len ); 
extern int    atoi ( const char *src ) ;
extern char*  itoa ( unsigned long decem,char *buf,unsigned char base ) ;// base can be 10,16,8
extern int    sprintf ( char* dbuf,const char* fmt,... );
extern int    vsprintf ( char* dbuf,const char* fmt,va_list va_p ) ;
extern unsigned char isnum ( char ch ) ;
extern long   strtol ( const char *src,char **err_pos,int base ); 

extern inline void * memcpy(void * dest,const void * src, unsigned long size);
extern inline void * memmove(void * dest,const void * src, int size);
extern inline int    memcmp(const void * cs,const void * ct,int count);
extern inline void * memchr(const void * cs,char c,int count);
extern inline void * memset(void * s,char c,int count);

extern int vsprintf ( char* dbuf,const char* fmt,va_list va_p ); 
#endif

