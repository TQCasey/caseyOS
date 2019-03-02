#include <casey/types.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <casey/config.h>
#include "fs.h"

int do_lseek(unsigned short pid,unsigned int fd,unsigned long offset, int origin)
{
	FILE_STRUCT * file;
	int tmp;

	if ( (fd >= MAX_FILPS) || !(file = filp(pid,fd)) || !(file->f_inode_ptr) ) 
		return (-1);
	switch (origin) 
    {
		case 0:         /* from start */
			if (offset < 0) return (-1);
			file->f_pos = offset;       
			break;
		case 1:         /* from current */
			if (file->f_pos+offset < 0) return (-1);
			file->f_pos += offset;
			break;
		case 2          /* from tail */:
			if ((tmp=file->f_inode_ptr->i_size+offset) < 0)
				return (-1);
			file->f_pos = tmp;
			break;
		default:
			return (-1);
	}
	return (file->f_pos);
}

int do_read (unsigned short pid,unsigned int fd,char * buf,int count)
{
	FILE_STRUCT * file;
	MEM_INODE * inode;

	if (fd >= MAX_FILPS || (count < 0) || !(file = filp (pid,fd)))
		return (-1);
	if (!count) return (0);
	inode = file->f_inode_ptr;

    /*if it is dir ,denied */
    if ( S_ISDIR(inode->i_mode))
    {
        printf ( "read dir inode denied !\n" );
        return (-1);
    }

    /* if dir mode or is regular file */
	if ( S_ISREG(inode->i_mode ))  
    {
        /* read left bytes */
		if (count+file->f_pos > inode->i_size)
			count = inode->i_size - file->f_pos;
		if (count <= 0) return (0);

		return (file_rd (inode,file,buf,count));
	}
	return (-1);
}

int do_write (unsigned short pid,unsigned int fd,char * buf,int count)
{
	FILE_STRUCT * file;
	MEM_INODE * inode;

	if (fd >= MAX_FILPS || (count < 0) || !(file = filp (pid,fd)))
		return (-1);
	if (!count) return (0);

	inode = file->f_inode_ptr;

    /*if it is dir ,denied */
    if ( S_ISDIR(inode->i_mode))
    {
        printf ( "write to dir inode denied !\n" );
        return (-1);
    }

    /* if dir mode or is regular file */
	if ( S_ISREG(inode->i_mode ))  
		return (file_wr(inode,file,buf,count));
	return (-1);
}
