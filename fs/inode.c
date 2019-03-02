#include <casey/types.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include "fs.h"
#include <sys/stat.h>
#include <casey/config.h>

#define NR_INODE    64  /* 2 block buffer size */
static MEM_INODE inode_table[NR_INODE] ; 

MEM_INODE *get_empty_inode ( void ) 
{
    int i = 0 ,j = 0;

    /* use cnt and dirt to judge if the inode is taken */
    while ( true )
    {
        for ( i = 0 , j = NR_INODE - 1; i < j ; i ++ , j -- ) 
        {
            /* just return the place ,not set cnt */
            if ( !inode_table[i].i_cnt && !inode_table[i].i_dirt ) 
                return (&inode_table[i]);
            if ( !inode_table[j].i_cnt && !inode_table[j].i_dirt ) 
                return (&inode_table[j]);
        }
        sync_dev_inodes ();
    }
    return (NULL);  /* not here ! */
}
/* read inode from dev */
static 
void read_dev_inode ( MEM_INODE *m_rd_inode ) 
{
    unsigned long inode_blk_nr;
    MEM_SUPER_BLOCK *sb ;

    if ( !m_rd_inode || !m_rd_inode->i_dev ) return ;
    if ( m_rd_inode->i_valid ) return ;/* no need to read inode from dev */

    if ( !(sb = get_sblk (m_rd_inode->i_dev)))
        crash ( "trying to read inode from nonexistent device ! \n" );
    /* get blk buffer index */
    inode_blk_nr = 1 + 1 + sb->sb_imap_blocks + sb->sb_zmap_blocks + 
        m_rd_inode->i_num / INODES_PER_BLK;

    getbuf (m_rd_inode,sizeof(DEV_INODE),/* src,len  */
            m_rd_inode->i_dev,inode_blk_nr, /* dev,blk_nr */
            32 * (m_rd_inode->i_num % INODES_PER_BLK) );    /* offset */
    m_rd_inode->i_valid= true;
}

/* write inode to dev ,NOTES: wb doesn't means free inode 
 * 'cause cnt may not be zero for it still counted by others
 * wb just sync inode and make inode clean !!
 */
static 
void write_dev_inode ( MEM_INODE *m_wr_inode ) 
{
    unsigned long inode_blk_nr;
    MEM_SUPER_BLOCK *sb ;

    if ( !m_wr_inode ) return ;
    if ( !m_wr_inode->i_dev || !m_wr_inode->i_dirt ) return ;

    if ( !(sb = get_sblk (m_wr_inode->i_dev)))
        crash ( "trying to write inode without device ! \n" );

    /* get blk buffer index */
    inode_blk_nr = 1 + 1 + sb->sb_imap_blocks + sb->sb_zmap_blocks + 
        m_wr_inode->i_num / INODES_PER_BLK;

    setbuf (m_wr_inode->i_dev,inode_blk_nr, /* dev,blk_nr */
            32 *(m_wr_inode->i_num%INODES_PER_BLK), /* offset */
            m_wr_inode,sizeof(DEV_INODE));  /* src,len */

    m_wr_inode->i_dirt = false;/* clear */
}

/* write inodes into buffers */
void sync_dev_inodes ( void  )
{
    int i = 0 , j = 0 ; 

    for ( i = 0 , j = NR_INODE - 1 ; i  <= j ; i ++ , j -- ) 
    {
        if ( inode_table[i].i_dirt && inode_table[i].i_valid )
            write_dev_inode (&inode_table[i]);
        if ( inode_table[j].i_dirt && inode_table[j].i_valid )
            write_dev_inode (&inode_table[j]);
    }
}

/* get a free inode from inode map */
MEM_INODE* iget ( unsigned short dev_no,unsigned long inode_nr )
{
    if (!dev_no) 
        crash ( "iget () with invalidate device !\n" );

    MEM_INODE *m_inode = NULL;
    int i = 0 , j = 0 ;

    /* first find in inode table */
    for ( i = 0 , j = NR_INODE - 1; i <= j ; i ++ , j -- ) 
    {
        /* just return the place ,not set cnt */
        if ( inode_table[i].i_dev == dev_no && inode_table[i].i_num == inode_nr ) 
        {
            inode_table[i].i_cnt ++;
            return (&inode_table[i]);
        }
        if ( inode_table[j].i_dev == dev_no && inode_table[j].i_num == inode_nr ) 
        {
            inode_table[j].i_cnt ++;
            return (&inode_table[j]);
        }
    }
    /* if not found */
    while ( true )
    {
        if ( m_inode = get_empty_inode () )  
        {
            memset (m_inode,0,sizeof(MEM_INODE));   /*clean ,invalidate */
            m_inode->i_cnt   = 1;
            m_inode->i_dev   = dev_no;
            m_inode->i_num   = inode_nr;
            read_dev_inode (m_inode);   /* read to m_inode in inode_tables */
            return (m_inode);       /* clean */
        }
        //sync_dev_inodes (); /* sync some inodes which are dirt */
    }
    crash ( "can not get more mem inode !\n" );
}

/* free a inode ,write back to dev */
void iput ( MEM_INODE *m_puti ) 
{
    if (!m_puti) return ;
    if (!m_puti->i_cnt ) /* use count to just if inode_table is empty */ 
        crash ( "trying to free free inode !\n" );
    if (!m_puti->i_dev)
    {
        m_puti->i_cnt --;
        return;
    }

    m_puti->i_cnt --;
    if ( m_puti->i_dirt )           /* modified ! */
    {
        write_dev_inode (m_puti);   /* this is real free at this moment ! */
        return ;
    }
    /* read only ! */
    if ( !m_puti->i_cnt )           /* clean && no cnt */
    {
        if (!m_puti->i_nlinks  )    /*clean && not cnt && no hard links */
        {
            printf ( "fuck u !\n" );
            clear_iblks (m_puti);   /* clear all this blocks */
            free_inode (m_puti);    /* no hard links ,we need to free it (write back) */
        }
    }
}

/* zero_mem all inode_tables */
inline void init_inodes ( void ) 
{
    memset (inode_table,0,sizeof(inode_table));
}

/* fcrt == true ,creat when not exist */
static 
unsigned long __bmap ( MEM_INODE *m_inode,unsigned long file_blk_no,bool fcrt,bool *created ) 
{
    if (!m_inode) return (0);

    BUFFER_HEAD *bh ;
    unsigned  long blk_no = 0;
    unsigned short *p = NULL;

    if (!m_inode->i_dev)
        crash ( "trying to __bmap blk to non-existent device ! file nr %d\n",
                file_blk_no );
    if (!m_inode->i_cnt)
        crash ( "trying to __bmap blk to invalidate inode (dev 0x%x,inr %d)!\n",
                m_inode->i_dev,m_inode->i_num ); 
    if (file_blk_no >=  7 + 512 + 512 * 512) 
        crash ( "__bmap : blk_no is too big !\n" );
    if (file_blk_no < 7)
    {
        if ( fcrt && !m_inode->i_zone[file_blk_no] )
        {
            if (m_inode->i_zone[file_blk_no] = new_block (m_inode->i_dev) ) 
            {
                *created = true;
                m_inode->i_crt_time = get_tks ();
                m_inode->i_dirt = true;
            }
        }
        return (m_inode->i_zone[file_blk_no]);
    }

    file_blk_no -= 7 ;
    if ( file_blk_no < 512 )
    {
        if (fcrt && !m_inode->i_zone[7] ) 
        {
            if ( m_inode->i_zone[7] = new_block (m_inode->i_dev) ) /* alloc a mem to save re_direct */
            {
                m_inode->i_dirt = true;
                m_inode->i_crt_time = get_tks ();
            }
        }
        if ( !m_inode->i_zone[7] ) /* if failed ! */
            return (0);
        if ( !(bh = bread (m_inode->i_dev,m_inode->i_zone[7])) ) 
            return (0);
            /* read  buffer,why read ?,not get_blk ? ,'cause it may be not create block ,
             * read the old one also ok 
             * */
        p = (unsigned short*)(bh->bh_buf);
        blk_no = p[file_blk_no];    /* this must be here,logical short cut ! */
        if ( fcrt && !blk_no ) 
        {
            if ( (blk_no = new_block (m_inode->i_dev)) )
            {
                *created = true;
                p[file_blk_no] = (unsigned short)blk_no;
                bh->bh_dirt = true;
            }
        }

        brelse (bh);
        return ( blk_no ) ;
    }
    
    file_blk_no -= 512 ;

    if ( fcrt && !m_inode->i_zone[8] )
    {
        if (m_inode->i_zone[8] = new_block (m_inode->i_dev))
        {
            m_inode->i_dirt = true;
            m_inode->i_crt_time = get_tks ();
        }
    }
    if (!m_inode->i_zone[8])
        return (0);
    if ( !(bh = bread (m_inode->i_dev,m_inode->i_zone[8])) ) 
        return (0);
    p = (unsigned short*)(bh->bh_buf);
    blk_no = p[file_blk_no>>9]; /* avoid loagical shut cut */ 

    if ( fcrt && !blk_no )
    {
        if ( blk_no = new_block (m_inode->i_dev) ) 
        {
            p[ file_blk_no >> 9] = blk_no ;
            bh->bh_dirt = true;
        }
    }

    brelse (bh);
    if ( !blk_no ) return (0);
    if ( !(bh = bread (m_inode->i_dev,blk_no)) ) 
        return (0);
    p = (unsigned short *)(bh->bh_buf);
    blk_no = p[file_blk_no%512];

    if ( !blk_no && fcrt ) 
    {
        if ( blk_no = new_block (m_inode->i_dev))
        {
            *created = true;
            p[file_blk_no % 512] = blk_no;
            bh->bh_dirt = true;
        }
    }
    brelse (bh);
    return (blk_no);
}

unsigned long bmap ( MEM_INODE *m_inode,unsigned long file_blk_no ) 
{
    unsigned char tmp;
    return (__bmap (m_inode,file_blk_no,false,&tmp));
}

unsigned long create_block ( MEM_INODE *m_inode,unsigned long file_blk_no ,bool *created ) 
{
    return (__bmap (m_inode,file_blk_no,true,created));
}

/* this is ONLY called by truncate */
static 
void free_ind ( unsigned short dev_no,unsigned long blk_no ) 
{
    if ( !dev_no || !blk_no ) return ;
    BUFFER_HEAD *bh;
    unsigned short *p = NULL,i = 0;

    if ( bh = bread (dev_no,blk_no) ) 
    {
        p = (unsigned short*)(bh->bh_buf);
        for ( i = 0 ; i < 512 ; i ++ ) 
            if (p[i]) free_block (dev_no,p[i]);
        brelse (bh);
    }
    free_block (dev_no,blk_no);
}

static 
void free_dind ( unsigned short dev_no,unsigned long blk_no ) 
{
    if ( !dev_no || !blk_no ) return ;
    BUFFER_HEAD *bh;
    unsigned short *p,i = 0 ;

    if ( bh = bread (dev_no,blk_no) )
    {
        p = (unsigned short*)(bh->bh_buf);
        for ( i = 0 ; i < 512 ; i ++ ) 
            if ( p[i] ) free_ind (dev_no,p[i]);
        brelse (bh);
    }
    free_block (dev_no,blk_no);
}

/* same as linux truncate() */
void clear_iblks ( MEM_INODE *m_inode ) 
{
    if (NULL == m_inode) return ;
    if (!m_inode->i_dev ) return ;
    if (!S_ISREG (m_inode->i_mode) || S_ISDIR(m_inode->i_mode) ) return;
    unsigned char i = 0;
    
    for ( i = 0 ;  i < 7 ; i ++ ) 
    {
        if (m_inode->i_zone[i])
        {
            free_block (m_inode->i_dev,m_inode->i_zone[i]);
            m_inode->i_zone[i] = 0;
        }
    }
    free_ind (m_inode->i_dev,m_inode->i_zone[7]);
    free_dind(m_inode->i_dev,m_inode->i_zone[8]);

    m_inode->i_zone[7] = m_inode->i_zone[8] = 0;
    m_inode->i_size = 0;
    m_inode->i_mtime = m_inode->i_acc_time = get_tks ();
}

void free_file_blk ( MEM_INODE *m_inode,unsigned long file_blk_no ) 
{
    if (!m_inode) return;

    BUFFER_HEAD *bh ;
    unsigned  long blk_no = 0;
    unsigned short *p = NULL;

    if (!m_inode->i_dev)
        crash ( "trying to free file blk to non-existent device ! file nr %d\n",
                file_blk_no );
    if (!m_inode->i_cnt)
        crash ( "trying to free from invalidate inode\n" ); 
    if (file_blk_no >=  7 + 512 + 512 * 512) 
        crash ( "free_file_blk : blk_no is too big !\n" );
    if (file_blk_no < 7)
    {
        free_block (m_inode->i_dev,m_inode->i_zone[file_blk_no]);
        m_inode->i_zone[file_blk_no] = 0;
        m_inode->i_dirt = true;
        return ;
    }

    file_blk_no -= 7 ;
    if ( file_blk_no < 512 )
    {
        if ( !m_inode->i_zone[7] ) /* if failed ! */
            return ;
        if ( !(bh = bread (m_inode->i_dev,m_inode->i_zone[7])) ) 
            return ;
        p = (unsigned short*)(bh->bh_buf);
        blk_no = p[file_blk_no];    /* this must be here,logical short cut ! */
        free_block (m_inode->i_dev,blk_no);
        p[file_blk_no] = 0;
        bh->bh_dirt = true;
        brelse (bh);
        return ;
    }
    
    file_blk_no -= 512 ;
    if (!m_inode->i_zone[8])
        return ;
    if ( !(bh = bread (m_inode->i_dev,m_inode->i_zone[8])) ) 
        return ;
    p = (unsigned short*)(bh->bh_buf);
    blk_no = p[file_blk_no>>9]; /* avoid loagical shut cut */ 
    brelse (bh);
    if ( !blk_no ) return ;
    if ( !(bh = bread (m_inode->i_dev,blk_no)) ) 
        return ;
    p = (unsigned short *)(bh->bh_buf);
    blk_no = p[file_blk_no%512];

    free_block (m_inode->i_dev,blk_no);
    p[file_blk_no%512] = 0;
    bh->bh_dirt = true;

    brelse (bh);
}
