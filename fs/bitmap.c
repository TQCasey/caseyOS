#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "fs.h"

/* in 1024 bytes buffer scale */
static 
unsigned char setb (unsigned char *addr,unsigned long nr)
{
    unsigned long res;
    __asm__ __volatile__("btsl %2,%3\n\tsetb %%al": 
                        "=a" (res):"0" (0),"r" (nr),"m" (*(addr)));

    return ((unsigned char)(res));
}

/* in 1024 bytes buffer scale */
static
unsigned char clrb (unsigned char *addr,unsigned long nr) 
{
    unsigned long res;
    __asm__ __volatile__("btrl %2,%3\n\tsetnb %%al": 
                        "=a" (res):"0" (0),"r" (nr),"m" (*(addr)));
    return ((unsigned char)(res));
}
 
/* find first zero bit offset from a block buffer */
inline unsigned long find_first_zero ( void *start ) 
{
    int i = 0 ,j = 0 ;
    char *p = (char*)start,code[8] = {0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x07F};

    /* this is not so good,I'll fix it ! */
    for ( i = 0; i < 1024 ;i ++ )
    {
        if ( 0xFF == p[i] ) continue;
        for ( j = 0 ; j < 8 ; j ++ ) 
            if ( code[j] == p[i] )
                return ( (i<<3) + j );
    }
    return (-1);
}

/* free block ,set clear  zmap */
void free_block ( unsigned short dev_no,unsigned long blk_nr ) 
{
    MEM_SUPER_BLOCK *sb = NULL;
    BUFFER_HEAD *tmp = NULL;

    if ( !(sb = get_sblk (dev_no)) ) 
        crash ( "trying to free block on nonexistent devive !\n" );
    if ( blk_nr < sb->sb_first_datazone || blk_nr >= sb->sb_nzones ) 
        crash ( "trying to free non-zoned zone !\n" ) ;
    
    /* if in hash ,free it */
    if (tmp = get_blk (dev_no,blk_nr))
    {
        if ( tmp->bh_cnt != 1 ) 
            crash ( "trying to free block %d,count = %d\n",
                    blk_nr,tmp->bh_cnt ) ;
        tmp->bh_dirt = false;   /* false == really free,add in free_list */
        tmp->bh_valid = false;   /* up */
        brelse (tmp);
    }

    if ( blk_nr < sb->sb_first_datazone ) 
        crash ( "free_block : blk_nr %d error !\n",blk_nr );
    blk_nr -= sb->sb_first_datazone;    /* data zone nr */

    if ( clrb (sb->sb_zmap_bh_ptr[blk_nr/8192]->bh_buf,blk_nr%8192) ) 
        crash ( "block bit map (blk_nr : %d) aready cleared !\n",blk_nr );
    /* just clear bit,the block buffer in hash is no more used again */
    sb->sb_zmap_bh_ptr[blk_nr/8192]->bh_dirt = 1;   /* bmap is dirt */
}

/* new a block set zmap ,return the block number */
unsigned long new_block ( unsigned short dev_no ) 
{
    MEM_SUPER_BLOCK *sb;
    BUFFER_HEAD *tmp ;
    int i = 0, j = 8192;

    if ( !(sb = get_sblk (dev_no)) ) 
        crash ("trying to new block from  nonexistent device !\n" );

    for ( i = 0 ; i < sb->sb_zmap_blocks ; i ++ ) 
    {
        if ( tmp = sb->sb_zmap_bh_ptr[i] ) 
            if ( (j = find_first_zero (tmp->bh_buf)) != (-1) )/* found one */ 
                break; /* if not found ,continue */
    }

    if ( i >= 8 || !tmp || j == (-1) ) return (0);

    if ( setb (tmp->bh_buf,j) ) 
        crash ( "new block: bit alreay set !\n");
    tmp->bh_dirt = true;        /* modified */
    /* i -- blks_offset ,j -- blk_bit_offset */
    j += i * 8192 + sb->sb_first_datazone;
    if ( j >= sb->sb_nzones ) return (0);

    if ( !(tmp = get_blk (dev_no,j)) ) 
        crash  ( "new block () : get blk failed !\n" );
    memset (tmp->bh_buf,0,BLK_SIZE);

    tmp->bh_dirt = true;
    tmp->bh_valid = true;   /* add in dirt_list ,'cause it always be write*/

    brelse (tmp);           /* new block okay ! */
    return (j);
}


/* new a device indode ,set imap,return the inode number 
 * almost the same with new block () 
 */
MEM_INODE* new_inode ( unsigned short dev_no ) 
{
    /* addr */
    MEM_SUPER_BLOCK *sb ;
    BUFFER_HEAD *imap;
    MEM_INODE *m_inode;

    int i = 0 , j = 8192 ;

    if ( !(sb = get_sblk (dev_no)) ) 
        crash ( "trying to new inode from nonexistent device !\n" ) ;

    for ( i = 0 ; i < sb->sb_imap_blocks ; i ++ ) 
    {
        if (imap = sb->sb_imap_bh_ptr[i] ) 
            if ( (j = find_first_zero (imap->bh_buf)) != (-1) ) break;
    }
    if ( i >= 8 || !imap || j == (-1) ) return (NULL);

    if ( setb (imap->bh_buf,j) ) 
        crash ( "new inode : bit alredy set !\n" );

    imap->bh_dirt = true;

    if ( !(m_inode = get_empty_inode ()) )
        crash ( "no enough inode table avaliable now !\n" );

    m_inode->i_cnt      = 1;
    m_inode->i_dirt     = true;
    m_inode->i_nlinks   = 1;
    m_inode->i_dev      = dev_no;
    m_inode->i_locked   = false;

    m_inode->i_uid      = 0;/*FIXME*/
    m_inode->i_gid      = 0;

    m_inode->i_num      = i * 8192 + j ;/* inode nr */
    m_inode->i_mtime    = m_inode->i_acc_time = m_inode->i_crt_time = get_tks ();
    return (m_inode);
}

/* free a devive inode ,clear imap */
void free_inode ( MEM_INODE *inode ) 
{
    if ( !inode ) return ;

    MEM_SUPER_BLOCK *sb ;
    BUFFER_HEAD *tmp ;

    if ( !(sb =  get_sblk (inode->i_dev)) ) 
        crash ( "trying to free inode from nonexistent device !\n" ) ;
    if ( !inode->i_dev ) {
        memset (inode,0,sizeof(MEM_INODE));
        return ;
    }
    if ( inode->i_cnt )
        crash ( "trying to free inode with count = %d \n",inode->i_cnt ) ;
    if ( inode->i_nlinks ) 
        crash ( "trying to free inode with links = %d \n",inode->i_nlinks );
    if ( !(sb = get_sblk (inode->i_dev)) )
        crash ( "trying to free nonexistent device inode !\n" );
    if ( !(tmp=sb->sb_imap_bh_ptr[inode->i_num/8192]) )
        crash ( "imap block %d not existent !\n",inode->i_num/8192 );
    if ( clrb (tmp->bh_buf,inode->i_num%8192) )
        crash ( "free_inode (): bit %d alredy cleared !\n",inode->i_num%8192 );
    tmp->bh_dirt = true;
    memset (inode,0,sizeof(MEM_INODE));/* call sync_bmaps () */
}
