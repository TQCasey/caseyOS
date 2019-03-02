/*--------------------------------------------------------------------
 * For assert
 * 2012 - 1 - 14
 * By Casey
 *--------------------------------------------------------------------*/

#ifndef __ASSERT_H__
#define __ASSERT_H__


#define ASSERT
#ifdef  ASSERT
extern void assert_fail (const char* expression,
                         const char* local_file,
                         const char* base_file,
                         unsigned long line_nr );
#define assert(exp)\
    if(!(exp))\
    {\
        assert_fail (#exp,__FILE__,__BASE_FILE__,__LINE__);\
    }/* if bad assert */
#else
#define assert(exp)   (void*)0
#endif

extern void spin ( const char *fmt,... ) ;
#endif
