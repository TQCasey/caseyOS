#ifndef __SYS_STAT_H__
#define __SYS_STAT_H__

/* inode imode use 15 - 12 to identify file type 
 * this is cpied from linux 0.11 ,no big changes !
 */

#define S_IFMT  00170000        /* file type maake 0xF000 */
#define S_IFREG  0100000        /* 0x8000  regular   */    
#define S_IFBLK  0060000        /* 0x6000  blk file  */
#define S_IFDIR  0040000        /* 0x4000  dir file  */
#define S_IFCHR  0020000        /* 0x2000  chr file  */
#define S_IFIFO  0010000        /* 0x1000  FIFO file */

/* 11 - 9 for file info */
#define S_ISUID  0004000        /* 0x800 is uid */
#define S_ISGID  0002000        /* 0x400 is gid */
#define S_ISVTX  0001000        /* 0x200 is vtx */

#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)

#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

#define I_TYPE          0170000
#define I_DIRECTORY	    0040000
#define I_REGULAR       0100000
#define I_BLOCK_SPECIAL 0060000
#define I_CHAR_SPECIAL  0020000
#define I_NAMED_PIPE	0010000
#define I_SET_UID_BIT   0004000
#define I_SET_GID_BIT   0002000

extern long mkdir ( const char *path ,unsigned short mode);;
extern long rmdir ( const char *path ); 

#endif
