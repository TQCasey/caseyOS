#include <casey/types.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include "fs.h"
#include <casey/mm.h>

/* (64M - 3M) to 64M - 2M is kernel 2M to 3M */
#define BUF_START       FSBUF
#define BUF_END         (FSBUF + 0x100000)       
#define NR_HASH         989                    /* hash_tables_nr == buffers_nr is ok ! */

static unsigned long NR_BUFFERS;
static BUFFER_HEAD *free_list  = NULL;         /* free_list */
static BUFFER_HEAD *hash[NR_HASH] = {0};       /* hash table */

#define get_irow(dev_no,blk_no)         (((dev_no)^(blk_no))%NR_HASH)
#define hash_row(dev_no,blk_no)         (hash[ get_irow (dev_no,blk_no)])

/* this hash *p must be single */
static 
BUFFER_HEAD* add_hash ( BUFFER_HEAD *p ) 
{  
    /* no dev ,invalidate buffer head  denied ! */
	if ( !p || !p->bh_dev )
        crash ( "error : wrong buffer argument !\n" );
    unsigned short dev_no = p->bh_dev;
    unsigned long  blk_no = p->bh_blk_nr;

	/* ok ! appened to the tail */
	if ( !(hash_row (dev_no,blk_no)) )	
    {/* if the first time add */
		p->hash_next = p;
		p->hash_prev = p;
		hash_row (dev_no,blk_no) = p;
	}
    else
    {
        /* appened to the tail */
        (hash_row (dev_no,blk_no))->hash_prev->hash_next = p;
        p->hash_prev = (hash_row (dev_no,blk_no))->hash_prev;
        (hash_row (dev_no,blk_no))->hash_prev = p;
        p->hash_next = hash_row (dev_no,blk_no);
    }
    return (p);
}

/* pick a free buffer ,add hash table */
BUFFER_HEAD *get_buffer ( unsigned short dev_no,unsigned long blk_no  )
{
	BUFFER_HEAD *ret = NULL;

    while ( true )
    {
        if (free_list) /* if not null */
        {
            ret = free_list->free_next;
            /* rm from free_list */
            if ( ret == free_list ) /* last node */
                free_list = NULL ;
            else
            {
                free_list->free_prev->free_next = free_list->free_next;
                free_list->free_next->free_prev = free_list->free_prev;
                ret = free_list;
                free_list = free_list->free_next;
            }
            /* add in hash */
            ret->free_next = ret->free_prev = NULL;
            ret->bh_dev = dev_no;
            ret->bh_blk_nr = blk_no;
            return ( add_hash (ret) ) ;
        }
        else
            sync_blks (dev_no,100);
    }
	return (ret);
}

/* find buffer if it is in hash list */
BUFFER_HEAD *find_buffer ( unsigned short dev_no,unsigned long blk_no ) 
{
	int i = get_irow(dev_no,blk_no);

    if ( !hash[i] ) return (NULL);

	BUFFER_HEAD *q = hash[i]->hash_prev ;	/* the tail */
	BUFFER_HEAD *p = hash[i];

    if ( !q )
        crash ( "buffer head chain is not correct now !\n" );

	/* start to search */
	while ( true ) {
		if ( dev_no == p->bh_dev && blk_no == p->bh_blk_nr ) 
            return (p);
		if ( dev_no == q->bh_dev && blk_no == q->bh_blk_nr )
            return (q);
		if ( p->hash_next == q ) break;		/* odd */
		if ( p == q ) break;				/* even */
		p = p->hash_next;
		q = q->hash_prev;					/* push */
	}
	return (NULL);
}

/* free a hash table ,add in free list */
void free_buffer  ( BUFFER_HEAD *p ) 
{
	if ( NULL == p ) return ;	/* null ptr */
	BUFFER_HEAD *q = hash_row (p->bh_dev,p->bh_blk_nr);		/* get hash_row of p */
    if (p->bh_cnt) 
        crash ( "trying to free buffer dev 0x%x,blk_no %d with count = %d !\n",
                p->bh_dev,p->bh_blk_nr,p->bh_cnt );
	if (!q) return ;

	if (q == p )	/* last hash node in this row*/
		hash_row (p->bh_dev,p->bh_blk_nr) = NULL;	/* clr hash head */
	else {
		/* rm hash */
		p->hash_prev->hash_next = p->hash_next;
		p->hash_next->hash_prev = p->hash_prev;
	}
	/* add in free_list tail */
	if ( !free_list ){   /* first add */
		p->free_next = p; p->free_prev = p;
		free_list = p;
	}
    else{
	    free_list->free_prev->free_next = p;
	    p->free_prev = free_list->free_prev;

	    p->free_next = free_list;
	    free_list->free_prev = p;
    }
    p->bh_cnt       = 0;
    p->bh_dirt      = false;    p->bh_valid = false;
    p->bh_locked    = false;    p->bh_wait  = NULL;
    p->bh_blk_nr    = 0;        p->bh_dev   = 0;
    p->hash_next    = p->hash_prev          = NULL;
    p = NULL;
}

/* buffers init */
void init_buffer ( void ) 
{
    BUFFER_HEAD *bh = (BUFFER_HEAD*)BUF_START;             /* ptr to first bh */
    unsigned char *buf = (unsigned char*)(BUF_END - BLK_SIZE);  /* ptr to first buffer */
    int i = 0;

    while ( (unsigned char*)(bh+1) <= (buf) ) {
        bh->bh_dev      = 0;
        bh->bh_dirt     = 0;
        bh->bh_locked   = false;
        bh->bh_wait     = 0;

        bh->bh_valid    = false;
        bh->bh_cnt      = 0;
        bh->bh_buf      = buf;  /* buf */
        bh->bh_blk_nr   = 0;    /* no block nr take this buffer block*/

        bh->free_prev   = bh-1; /* this will be ok when the fisrt node ,no page fault */
        bh->free_next   = bh+1; /* ptr to next one */

        bh ++;                  /* next bh */
        buf -= BLK_SIZE;        /* next blk */
        NR_BUFFERS ++;          /* blocks inc */
    }
    /* we need circle it */
    free_list = (BUFFER_HEAD*)BUF_START;        /* free_list ptr first bh */
    free_list->free_prev = --bh;                /* ptr the last bh */
    bh->free_next = free_list;                  

    for ( i = 0 ; i < NR_HASH ; i ++ ) hash[i] = NULL;

    printf ( "total buffer blocks : %d \n",NR_BUFFERS );
}

void sync_bmaps ( unsigned short dev_no )
{
    MEM_SUPER_BLOCK *sb = get_sblk (dev_no);/*find super block ,not count */
    if ( !sb ) return ;
    /* start */
    unsigned long i = 0 ,blk_no = 2;

    for ( i = 0 ; i < sb->sb_imap_blocks ; i ++,blk_no ++ ) 
    {
        if ( sb->sb_imap_bh_ptr[i] && sb->sb_imap_bh_ptr[i]->bh_dirt )
        {
            /* write back right away !*/
            ll_rw_blk (WRITE,dev_no,blk_no,1,sb->sb_imap_bh_ptr[i]->bh_buf);
            sb->sb_imap_bh_ptr[i]->bh_dirt = false;
        }
    }

    for ( i = 0 ; i < sb->sb_zmap_blocks ; i ++,blk_no ++ ) 
    {
        if ( sb->sb_zmap_bh_ptr[i] && sb->sb_zmap_bh_ptr[i]->bh_dirt )
        {
            /* write back right away !*/
            ll_rw_blk (WRITE,dev_no,blk_no,1,sb->sb_zmap_bh_ptr[i]->bh_buf);
            sb->sb_zmap_bh_ptr[i]->bh_dirt = false;
        }
    }
}

/* syc_block number */
unsigned char sync_blks ( unsigned short dev_no,unsigned long sync_blk_nr ) 
{
    long i = 0 ,j = 0 ;
    BUFFER_HEAD *bh = (BUFFER_HEAD*)BUF_START;
    unsigned long cnt = 0 ;

    for ( i = 0, j = NR_BUFFERS - 1 ; i <= j  ; i ++, j --  ) 
    {
        if ( bh[i].bh_dirt && bh[i].bh_valid 
          && bh[i].bh_dev == dev_no ) 
        {
            ll_rw_blk (WRITE,bh[i].bh_dev,bh[i].bh_blk_nr,1,bh[i].bh_buf);
            if ( !bh[i].bh_cnt ) 
                free_buffer ( &bh[i] ) ;
            if (cnt ++ >= sync_blk_nr ) break;
            bh[i].bh_dirt = false;
        }
        if ( bh[j].bh_dirt && bh[j].bh_valid 
          && bh[j].bh_dev == dev_no ) 
        {
            ll_rw_blk (WRITE,bh[j].bh_dev,bh[j].bh_blk_nr,1,bh[j].bh_buf);
            if ( !bh[j].bh_cnt ) 
                free_buffer ( &bh[j] ) ;
            if (cnt ++ >= sync_blk_nr ) break;
            bh[j].bh_dirt = false;
        }
    }
    return (1);
}
