#ifndef __FCNTL_H__
#define __FCNTL_H__

#define O_ACCMODE	0x0003
#define O_RDONLY	0x0000
#define O_WRONLY	0x0001
#define O_RDWR		0x0002
#define O_CREAT		0x0040	/* not fcntl */
#define O_EXCL		0x0080	/* not fcntl */
#define O_NOCTTY	0x0100	/* not fcntl */
#define O_TRUNC		0x0200	/* not fcntl */
#define O_APPEND	0x0400
#define O_NONBLOCK	0x0800	/* not fcntl */
#define O_NDELAY	O_NONBLOCK

#endif
