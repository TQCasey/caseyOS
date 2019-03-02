#include <casey/types.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include "fs.h"
#include <sys/stat.h>
#include <casey/config.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>

unsigned long file_rd(MEM_INODE * inode, FILE_STRUCT* this_file, char * buf, int count)
{
	int left,chars,nr;
	BUFFER_HEAD * bh;

    if ( !this_file || ((left = count) <= 0) || !buf ) return (0);

	while (left > 0) {
		if (nr = bmap(inode,(this_file->f_pos)/BLK_SIZE)){ 
			if (!(bh=bread(inode->i_dev,nr)))
				break;
        }
        else 
            bh = NULL;

		nr = this_file->f_pos % BLK_SIZE;
		chars = Min (BLK_SIZE-nr,left);
		this_file->f_pos += chars;
		left -= chars;
		if (bh) 
        {
			char * p = nr + bh->bh_buf;
			while (chars-->0)
                *buf ++ = *p ++;
			brelse(bh);
		} 
        else 
        {
            /* bread may failed ,however blk_no is okay ! */
			while (chars-->0)
                *buf ++ = 0x00;
		}
	}
	inode->i_acc_time = get_tks ();
	return ((count-left)?(count-left):0);
}

unsigned long file_wr ( MEM_INODE *m_wr,
        FILE_STRUCT *this_file,char *buf,long cnt ) 
{
    if ( !this_file || (cnt<0) || !buf ) return (0);

    __u64 pos = 0;

    if ( this_file->f_flags & O_APPEND ) /* attatch file */
        pos = m_wr->i_size;     /* move file ptr to the tail */
    else
        pos = this_file->f_pos; /* pos  */

    BUFFER_HEAD *bh;
    unsigned long blk_no = 0,offset = 0;
    long chars = 0,written = 0;
    char *p = NULL,created = false;

    while (cnt > 0) {
        if ( !(blk_no = create_block (m_wr,(this_file->f_pos/BLK_SIZE),&created))) 
            break;
        if ( !(bh = bread (m_wr->i_dev,blk_no)) ) 
            break;

        offset = pos % BLK_SIZE;
        chars = BLK_SIZE- offset;   /* total chars can be put in */
        if ( chars > cnt ) 
            chars = cnt ;
        p = bh->bh_buf + offset ;   /* ptr to buffer start */

        /* any way ,append file here will increase file size,... 
         * file size will be changed at this moment ,this will happened 
         * if not append but wrtie more size (> isize) bytes into 
         * this file ,we need to renew file size just once ( later ) 
         */
        pos += chars ;
        if ( pos > m_wr->i_size ) {
            m_wr->i_size = pos;
            m_wr->i_dirt = true;
        }

        cnt -= chars ;
        written += chars ;
        this_file->f_pos += chars;

        while ( chars -- > 0 ) 
            *p ++ = *buf ++;

        bh->bh_dirt = true;
        brelse (bh);
    }

    m_wr->i_mtime = get_tks ();

    if ( !(this_file->f_flags & O_APPEND )) {   /* if it is append,size already changed 
                                                 * so no need to change again *
                                                 */
        this_file->f_pos = pos;
        m_wr->i_crt_time = get_tks ();
    }

    return (written);
}
