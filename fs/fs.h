/* ----------------------------------------------------------------------------------
 * for file system 
 * 2012 - 3- 29 
 * By Casey 
 * ---------------------------------------------------------------------------------*/
#ifndef __FS_H__
#define __FS_H__

#include <casey/types.h>
#include <unistd.h>

/* buffer head */
typedef struct tagBUFFER_HEAD{
    __byte*     bh_buf ;                    // point to a block 
    __u32       bh_blk_nr;                  // block nr
    __u16       bh_dev;                     // buffer head device 
    __byte      bh_valid;                   // data valid bool val
    __byte      bh_dirt;                    // data modified bool val
    __byte      bh_cnt;                     // how many user using this blk?
    __byte      bh_locked;                  // buffer head locked bool val
    void*       bh_wait;                    // wait ptr
    struct      tagBUFFER_HEAD  *hash_prev; // hash prev ptr
    struct      tagBUFFER_HEAD  *hash_next; // hash next ptr 
    struct      tagBUFFER_HEAD  *free_prev; // free_list prev ptr 
    struct      tagBUFFER_HEAD  *free_next; // free_list next ptr 
}BUFFER_HEAD,*LPBUFFER_HEAD;

/* dev inode struct def here*/  /* must be 32 bytes */
typedef struct tagDEV_INODE{
    __u16       i_mode;                     // rwx
    __u16       i_uid;                      // user id 
    __u32       i_size;                     // file size ( inbytes )
    __u32       i_mtime;                    // latest modified time
    __u8        i_gid;                      // group id
    __u8        i_nlinks;                   // links for files
    __u16       i_zone[9];                  // 0 - 6 for direct fetch ,7 for 1st redirect fetch ,8 for 2nd...
}DEV_INODE,*LPDEV_INODE;

/* memory inode struct def here */
typedef struct tagMEM_INODE{
    __u16       i_mode;                     // rwx
    __u16       i_uid;                      // user id 
    __u32       i_size;                     // file size ( inbytes )
    __u32       i_mtime;                    // latest modified time
    __u8        i_gid;                      // group id
    __u8        i_nlinks;                   // links for files (different file name use the same inode
    __u16       i_zone[9];                  // 0 - 6 for direct fetch ,7 for 1st redirect fetch ,8 for 2nd...
    /* these are ONLY in memory */
    __u32       *i_wait;
    __u32       i_acc_time;                 // latest access time
    __u32       i_crt_time;                 // create time
    __u16       i_dev;                      // locate dev
    __u16       i_num;                      // inode nr
    __u16       i_cnt;                      // inode used times
    __u8        i_locked;                   // inode locked ?
    __u8        i_dirt;                     // inode dirt flag
    __u8        i_pipe;                     // pipe sig
    __byte      i_mounted;                  // already mounted?
    __byte      i_seek;                     // seek sig
    __byte      i_valid;                    // valid flag
}MEM_INODE,*LPMEM_INODE;

/* file_struct defs here */
#define MAX_OPEN_FILES                      64
#define MAX_FILPS                           20
typedef struct tagFILE_STRUCT{
    __u16       f_mode;                     // file opt mode
    __u16       f_flags;                    // file opt flag ( open / rw )
    __u16       f_cnt;                      // file descriptors count
    MEM_INODE   *f_inode_ptr;               // ptr to inode nr 
    __u64       f_pos;                      // file pos 
}FILE_STRUCT,*PFILE_STRUCT;


/* dev super block defs here */
typedef struct tagDEV_SUPER_BLOCK{
    __u16       sb_ninodes;                  // total inodes 
    __u16       sb_nzones;                   // totol logic blks 
    __u16       sb_imap_blocks;              // inode map blks 
    __u16       sb_zmap_blocks;              // logical map blks 
    __u16       sb_first_datazone;           // first data logic blk nr
    __u16       sb_log_zone_size;            // log2(data_blks/logic blks)
    __u32       sb_max_size;                 // file max size
    __u16       sb_magic;                    // fs magic num
}DEV_SUPER_BLOCK,*LPDEV_SUPER_BLOCK;


#define IMAP_SLOTS      8
#define ZMAP_SLOTS      8
/* mem super block defs here */
typedef struct tagMEM_SUPER_BLOCK{
    __u16       sb_ninodes;                  // total inodes 
    __u16       sb_nzones;                   // totol logic blks 
    __u16       sb_imap_blocks;              // inode map blks 
    __u16       sb_zmap_blocks;              // logical map blks 
    __u16       sb_first_datazone;           // first data logic blk nr
    __u16       sb_log_zone_size;            // log2(data_blks/logic blks)
    __u32       sb_max_size;                 // file max size
    __u16       sb_magic;                    // fs magic num
    /* these are ONLY in memory*/
    BUFFER_HEAD *sb_imap_bh_ptr[IMAP_SLOTS]; // imap buffer head ptr
    BUFFER_HEAD *sb_zmap_bh_ptr[ZMAP_SLOTS]; // zmap buffer head ptr
    __u16       sb_dev;                      // dev num
    MEM_INODE   *sb_i_nr;                    // mounted fs root dir inode nr
    MEM_INODE   *sb_i_mounted;               // mounted fs inode nr
    __u32       sb_time;                     // modified time
    __byte      sb_rd_only;                  // read only flag
    __byte      sb_dirt;                     // modified flag
    __u8        sb_locked;                   // locked ?
    __u32*      sb_wait;                     // wait ptr
}MEM_SUPER_BLOCK,*LPMEM_SUPER_BLOCK;


#define NAME_LEN                             14
/* struct dir entry */
typedef struct tagDIR_ENTRY{
    __u16       inode_nr;                    // inode nr
    char        name[ NAME_LEN ] ;           // name
}DIR_ENTRY,*LPDIR_ENTRY;


#define BLK_SIZE    1024
#define READ        1
#define WRITE       0
#define INODES_PER_BLK  32
#define ENTRIES_PER_BLK 64


#define FS_MAIN     0
#define FS_SYNC     1

#define ROOT_NR     1
#define ROOT_IND_NR 2


#define CASEY_MAGIC                 0x99

extern FILE_STRUCT file_table[];
extern PFILE_STRUCT filp_ptr[];  

#define filp(pid,fd)\
    (filp_ptr[(pid)*20 + (fd)])

extern 
unsigned long ll_rw_phys_sect ( unsigned char rw_rq,                  /* request r/w */
                                unsigned char drv_no,                 /* hard disk nr */
                                unsigned long sect_no,                /* from sect nr */
                                unsigned long sects_nr,               /* bytes in cnt */
                                void*         dest );                 /* buffer ptr */

extern 
unsigned long ll_rw_blk ( unsigned char  rw_rq,                       /* request r/w */
                          unsigned short dev_no,                      /* dev no */
                          unsigned long  blk_no,                      /* block nr */
                          unsigned long  blks_nr,                     /* blocks cnt */
                          void *         dest );

extern BUFFER_HEAD *get_buffer ( unsigned short dev_no,unsigned long blk_no );
extern void free_buffer  ( BUFFER_HEAD *p ); 
extern BUFFER_HEAD *find_buffer (unsigned short dev_no,unsigned long blk_no );

extern void brelse ( BUFFER_HEAD *bh_free );
extern BUFFER_HEAD *bread ( unsigned short dev_no,unsigned long blk_no ) ;
extern BUFFER_HEAD *get_blk ( unsigned short dev_no,unsigned long blk_no ); 
extern void sync_bmaps ( unsigned short dev_no );
extern unsigned char sync_blks ( unsigned short dev_no,unsigned long sync_blk_nr );
extern void sync_dev_inodes ( void  ); 

extern void* bread_page ( unsigned long addr,unsigned short dev_no,unsigned long blk_no[4] ); 
extern BUFFER_HEAD* breada ( unsigned short dev_no,unsigned long blk_no ,...); 

extern MEM_SUPER_BLOCK* read_sblk ( unsigned short dev_no );
extern MEM_SUPER_BLOCK* get_sblk ( unsigned short dev_no ) ;
extern void put_sblk ( unsigned short dev_no ); 
extern unsigned char get_part_info ( unsigned short dev_no,unsigned long *base_sect,
                            unsigned long *sects_nr ); 
extern unsigned long new_block ( unsigned short dev_no ); 
extern void free_block ( unsigned short dev_no,unsigned long blk_nr ) ;

extern void iput ( MEM_INODE *m_puti );
extern  MEM_INODE* iget ( unsigned short dev_no,unsigned long inode_nr );

extern unsigned long bmap ( MEM_INODE *m_inode,unsigned long file_blk_no ); 
extern unsigned long create_block (MEM_INODE *m_inode,unsigned long file_blk_no,bool *created);

extern void clear_iblks ( MEM_INODE *m_inode ); 
extern void free_file_blk ( MEM_INODE *m_inode,unsigned long file_blk_no ); 

extern MEM_INODE *namei (const char *path  );
extern MEM_INODE *get_empty_inode ( void ); 

extern MEM_INODE* new_inode ( unsigned short dev_no ); 

#endif
