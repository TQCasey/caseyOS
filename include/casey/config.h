/*-----------------------------------------------------------------------------------------------
 * for configurations 
 * 2012 - 05 - 16 
 * by casey 
 * ----------------------------------------------------------------------------------------------*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define DEV_NULL            0           /* not used */
#define DEV_MEM             1           /* ram disk */
#define DEV_FD              2           /* floppy */
#define DEV_HD              3           /* hard disk */
#define DEV_TTYX            4           /* ttyx */
#define DEV_TTY             5           /* tty local */
#define DEV_LP              6           /* printer */

#define MAX_DEVS            7

/* make dev number */
#define MAJOR_SHFT          8
#define MAKE_DEV(major_no,minor_no) (((major_no)<<MAJOR_SHFT)|(minor_no))

/* get major_dev_num & minor_dev_num */
#define MAJOR(dev_no)       (((dev_no)>>MAJOR_SHFT)&0xFF)
#define MINOR(dev_no)       ((dev_no) & 0xFF ) 

#define ROOT_DEV            0x305

#endif
