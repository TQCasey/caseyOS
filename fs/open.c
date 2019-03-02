#include <casey/types.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <casey/config.h>
#include "fs.h"

int do_open (unsigned short pid,const char *path,int flag,int mode)
{
    int fd = 0;
    FILE_STRUCT *ftbl = file_table;

    for ( fd = 0; fd < MAX_FILPS; fd ++ ) 
    {
        if (!filp (pid,fd) )
            break;
    }

    if ( fd >= MAX_FILPS ) 
    {
        printf ( "error : can not find empty filp !\n" );
        return (-1);
    }

    for (  ; ftbl < file_table + MAX_OPEN_FILES ; ftbl ++ )
    {
        if (!ftbl->f_cnt)
            break;
    }

    if (ftbl >= file_table + MAX_OPEN_FILES)
    {
        printf ( "error : can not find empty file_table !\n" );
        return (-1);
    }

    (filp(pid,fd) = ftbl)->f_cnt ++;

    MEM_INODE *m_inode;
    int ret = 0;
    if ( (ret = open_namei (path,flag,mode,&m_inode)) < 0 )
    {
        /* failed ! */
        filp (pid,fd) = NULL;
        ftbl->f_cnt = 0;
        return (ret);
    }

    ftbl->f_mode = m_inode->i_mode;
    ftbl->f_flags = flag;
    ftbl->f_inode_ptr = m_inode;
    ftbl->f_pos = 0;
    ftbl->f_cnt = 1;
    return (fd);
}

/* close file discriptor */
int do_close (unsigned short pid,int fd)
{
    if ( fd >= MAX_FILPS ) return (-1);

    FILE_STRUCT *this_file = filp (pid,fd);

    /* invlidate mem inodes here ! */
    if ( this_file == NULL ) return (-1);

    filp(pid,fd) = NULL;

    if ( !this_file->f_cnt ) 
        crash ( "pid %d,fd %d file cnt is already zero !\n",pid,fd );
    if ( --this_file->f_cnt ) 
        return (0);
    iput (this_file->f_inode_ptr);
    return (0);
}

int do_dup (int fd)
{
    return (fd);
}
