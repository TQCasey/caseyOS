#include <casey/types.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include "fs.h"
#include <sys/stat.h>
#include <casey/config.h>


/* get src byte from one buffer */
void* getbuf ( void *buf,unsigned long len,unsigned short dev_no,
        unsigned long blk_nr ,unsigned long offset ) 
{
    BUFFER_HEAD *bh;

    if ( !buf || !dev_no || (offset  + len) > 1024 ) 
        crash ( "get buf argument wrong !\n" );

    if ( !(bh = bread (dev_no,blk_nr)))
        crash ( "get buffer failed !\n" );
    memcpy (buf,bh->bh_buf + offset,len);
    brelse (bh);    /* free */

    return (buf);
}


/* set one buffer content */
void* setbuf ( unsigned short dev_no,unsigned long blk_nr,
        unsigned long offset,void *src,unsigned long len ) 
{
    BUFFER_HEAD *bh ;
    if ( !src || !dev_no || (offset + len) > 1024 ) 
        crash ( "set buf argument wrong !\n" );

    if ( !(bh = bread (dev_no,blk_nr)) )
        crash ( "set buffer failed !\n" );
    memcpy (bh->bh_buf + offset,src,len);
    bh->bh_dirt = true;
    brelse (bh);

    return (src);
}

/* block write use u64 to identify the dev blk pos*/ 
unsigned long blk_wr ( unsigned short dev_no,           /* dev nr */
                       __u64    *pos,char *src,         /* pos , src buf */
                       long cnt )                       /* bytes count */
{
    if ( !src || (cnt<0) || !dev_no ) return (0);

    unsigned long blk_no = (*pos)/BLK_SIZE,offset = (*pos) % BLK_SIZE;
    int chars = 0;
    unsigned long written = 0;
    BUFFER_HEAD *bh;
    char *p = NULL;

    while ( cnt > 0 ) {
        chars = BLK_SIZE - offset ;
        if ( chars > cnt )  /* less BLOCK_SIZE */
            chars = cnt;    
        if ( BLK_SIZE == chars ) /* one block */
            bh = get_blk (dev_no,blk_no);
        else
            bh = breada (dev_no,blk_no,blk_no+1,blk_no+2,-1);
        blk_no ++ ;
        if ( !bh ) 
            return (written);
        p = bh->bh_buf + offset ;
        offset = 0;
        *pos += chars ;
        written += chars ;
        cnt -= chars ;
        
        while ( chars -- > 0 ) 
            *p ++ = *src ++;
        bh->bh_dirt = true;
        brelse (bh);
    }
    return (written);
}

/* block read */

unsigned long blk_rd ( unsigned short dev_no,           /* dev nr */
                       __u64 *pos,char *dest,           /* pos ,dest  buf */
                       long cnt )                       /* bytes count */
{
    if ( !dest || (cnt<0) || !dev_no ) return (0);

    unsigned long blk_no = (*pos)/BLK_SIZE,offset = (*pos) % BLK_SIZE;
    int chars = 0;
    unsigned long read = 0;
    BUFFER_HEAD *bh;
    char *p = NULL;

    while ( cnt > 0 ) {
        chars = BLK_SIZE - offset ;
        if ( chars > cnt )  /* less BLOCK_SIZE */
            chars = cnt;    

        bh = breada (dev_no,blk_no,blk_no+1,blk_no+2,-1);
        blk_no ++ ;
        if ( !bh ) 
            return (read);
        p = bh->bh_buf + offset ;
        offset = 0;
        *pos += chars ;
        read += chars ;
        cnt -= chars ;
        
        while ( chars -- > 0 ) 
            *dest ++ = *p ++;
        brelse (bh);
    }
    return (read);
}
