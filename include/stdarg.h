/*--------------------------------------------------------
* for variable args
* 2011-12-10
* By Casey
*--------------------------------------------------------*/
#ifndef __ARG_H__
#define __ARG_H__

typedef char* 	    	va_list;
#define _INTSIZEOF(n)   ((sizeof(n)+sizeof(int)-1)&~(sizeof(int) - 1) ) 
#define va_start(ap,v) 	( ap = (va_list)&v + _INTSIZEOF(v) )              // ap -- > first arg
#define va_arg(ap,t) 	( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) ) // ap -- > next arg
#define va_end(ap)    	( ap = (va_list)0 )                               // disable ap

#endif 
