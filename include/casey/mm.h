/* -------------------------------------------------------------------------------------
 * for memory managerment
 * 2012 - 4 - 9 
 * By Casey
 * ------------------------------------------------------------------------------------*/

#ifndef __MM_H__
#define __MM_H__


#define PROCS_STACK_SIZE    (0x8000)
#define USER_SPACE          (0x4000000)


/* process mem map */
#define PROC_STACK          (USER_SPACE - 0x100000)         /* 63M - 64M */
#define PMBOX               (PROC_STACK - 0x8000)           /* 32K for private mail box */
#define CMBOX               (PMBOX  - 0x8000)               /* 32K for common mail box */
#define ROCKECT             (CMBOX  - 0x1000)               /* 4K for kernel && task comm buf */

/* for fs cache buffer */
#define FSBUF               (USER_SPACE - 0x300000)         /* for fs cache buffer */
#define FILPS_BUF           (ROCKECT - 0x7000)              /* 28 KB for filp structs */

/* terminal cache buffer */
#define STDIN_BUF           (ROCKECT - 0x1000)              /* 4KB for stdin buffer */
#define STDOUT_BUF          (STDIN_BUF - 0x8000)            /* 32KB for VGA buffer */

#endif
