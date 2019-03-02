#include <unistd.h>             /* syscalls for user */
#include <stdio.h>
#include "fs.h"
#include <assert.h>
#include <casey/config.h>
#include <sys/stat.h>
#include <sys/msgno.h>
#include <casey/sched.h>
#include <fcntl.h>


/* this is rw thread for hard disk .ONLY intr can wake it up */
extern void init_buffer ( void ) ;
extern void hd_info_init ( void ) ;
extern char load_root ( void );
extern void init_inodes ( void ); 


static 
void init_filps ( void )
{
    memset (file_table,0,MAX_OPEN_FILES * 4 );/* file_tables */
    memset (&filp_ptr,0,MAX_FILPS * MAX_OPEN_FILES * 4);/* file_tables */
}

#define InitFS() \
{\
    init_buffer ();\
    hd_info_init ();\
    init_inodes ();\
    load_root ();\
    init_filps ();\
    printf ( "Task fs init done ! \n" );\
} 

#define sync_all()\
{\
    sync_bmaps (ROOT_DEV);\
    sync_dev_inodes ();\
    sync_blks (ROOT_DEV,-1);\
}

static 
void list ( int argc,char argv[][30] ) 
{
    const char *path = argv[1];
    if ( NULL == path ) return;

    BUFFER_HEAD *bh;
    MEM_INODE *m_dir = namei (path);

    /* invalidate path */
    if ( NULL == m_dir ) {
        printf ( "error : dir not exsist !\n" );
        return ;
    }
    /* not dir inode */
    if ( !S_ISDIR (m_dir->i_mode)) {
        printf ( "error : not dir inode !\n" );
        iput (m_dir);
        return ;
    }

    long    i = 0,j = 0,k = 0,blk_no,blk_nr = m_dir->i_size/BLK_SIZE + 
                          (0 != (m_dir->i_size%BLK_SIZE)); 
    DIR_ENTRY *de_ptr = NULL;

    MEM_INODE *m_inode;

    bool f_show_i = 0,f_show_links = 0,f_show_cnt = 0,f_show_v = 0;

    for ( k = 2 ; k < argc ; k ++ ) 
    {
        if ( !strcmp (argv[k],"-l") )  f_show_links = 1;
        else if ( !strcmp (argv[k],"-i")) f_show_i = 1;
        else if ( !strcmp (argv[k],"-n")) f_show_cnt = 1;
        else if ( !strcmp (argv[k],"-v")) f_show_v = 1;
    }

    for ( i = 0 ; i < (512 * 512 +512 + 7)  && blk_nr > 0  ; i ++ ) /* enumenate all blks */
    {
        if ( !(blk_no = bmap (m_dir,i)) ) continue ; /* next block */

        bh = bread (m_dir->i_dev,blk_no);
        blk_nr --;
        if ( !bh ) continue;

        de_ptr = (DIR_ENTRY*)(bh->bh_buf);      /* start search in this buffer */

        for ( j = 0 ; j < ENTRIES_PER_BLK ; j ++ ) 
        {
            if ( !de_ptr[j].name[0] ) continue ;
            if ( (m_inode = iget (m_dir->i_dev,de_ptr[j].inode_nr)) )
            {
                if ( S_ISDIR (m_inode->i_mode)) 
                    printf ( "%14s/ ",de_ptr[j].name );
                else
                    printf ( "%14s ",de_ptr[j].name );
                iput (m_inode);
                if ( f_show_i )     printf ( "ind_nr %-4d ",m_inode->i_num );
                if ( f_show_links ) printf ( "lnk_nr %-4d ",m_inode->i_nlinks );
                if ( f_show_cnt )   printf ( "cnt    %-4d ",m_inode->i_cnt );
                if ( f_show_v )     putchar ('\n');
            }
        }
        brelse (bh);
    }
    iput (m_dir);
}

/* main thread for Service */
int main ( void ) 
{   
    InitFS ();

    delay_ms (1000);

    MAIL m;
    m.m_msg = 1111;
    m.m_dest_pid    = INIT;
    m.m_level       = MPL_USR;
    m.m_dest_tid    = 0;

    int i = 0;
    while ( true ) {
        delay_ms (10);
        if ( postm (&m) >= 0 ) 
            printf ( "(%d)" ,i++ );
    }

    /*
    char    a[100] = {0};
    int     cmd_cnt = 0,i = 0;
    char    argv[30][30] = {0};

    MAIL m;

    while ( true ) 
    {
        pickm (&m);
        switch (m.umsg)
        {
            case FSM_READ:
                printf ( "read command !\n" );
                replym (&m);
                break;
            case FSM_EXEC:
                memset (argv,0,900);
                m.pkg[0] = (unsigned long)argv;
                m.pkg[1] = 900;
                replym (&m);

                char (*p)[30] = argv;

                int argc = m.pkg[2];

                printf ( "\n" );

                while ( argc -- ) 
                    printf ( "%s ",*p ++ );

                printf ( "\n" );
                break;
        }
    }
    */

    while ( true ) pause ();
}
