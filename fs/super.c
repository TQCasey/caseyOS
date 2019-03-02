#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "fs.h"
#include <casey/config.h>
#include <sys/stat.h>

#define NR_SBLK     8
static MEM_SUPER_BLOCK super_blk [NR_SBLK];

/* return sblk index of super_blk array */
MEM_SUPER_BLOCK* get_sblk ( unsigned short dev_no ) 
{
    if ( !dev_no ) return (NULL); 
    int i = 0 ;

    for ( i = 0; i < NR_SBLK ; i ++ )
    {
        if ( super_blk[i].sb_dev == dev_no ) 
        {
            return (super_blk+i);
        }
    }
    return (NULL);
}

/* write back super block */
void put_sblk ( unsigned short dev_no ) 
{
    /* free this bh */
    if ( dev_no == ROOT_DEV ) {
        printf ( "Denied : trying to remove root_dev !\n" );
        return ;
    }

    MEM_SUPER_BLOCK *sblk = NULL;
    if ( !dev_no || !(sblk = get_sblk(dev_no)) ) return ;

    if (sblk->sb_i_mounted){
        printf ( "Denied : trying to remove mountical dev !\n" );
        return ;
    }

    if ( sblk->sb_dirt ) { /* if super block is modifed */  
        BUFFER_HEAD *tmp = get_blk (dev_no,1);  /* do not read sect */
        memcpy (tmp->bh_buf,sblk,sizeof(DEV_SUPER_BLOCK));
        tmp->bh_dirt = true;
        tmp->bh_valid = true;
        brelse (tmp);   /* add in dirt list */
    }

    int i = 0 ;
    sblk->sb_dev = 0;
    for ( i = 0 ; i < ZMAP_SLOTS ; i ++ )
        brelse (sblk->sb_zmap_bh_ptr[i]);   /*add in dirt */
    for ( i = 0 ; i < IMAP_SLOTS ; i ++ )   
        brelse (sblk->sb_imap_bh_ptr[i]);   /* add in dirt */
}

MEM_SUPER_BLOCK* read_sblk ( unsigned short dev_no )
{
    int i = 0 ;

    for ( i = 0 ; i < NR_SBLK ; i ++ ) 
    {
        if (!super_blk[i].sb_dev)
        {
            BUFFER_HEAD *tmp = bread (dev_no,1);    /* validate,not dirt  */
            if ( !tmp ) 
                crash ( "get super error ! at device 0x%x\n",dev_no );
            memcpy (super_blk+i,tmp->bh_buf,sizeof(MEM_SUPER_BLOCK));
            brelse (tmp);//free buffer 
            return (super_blk+i);   
        }
    }
    printf ( "error: more than %d fs mounted !\n",NR_SBLK );
    return (NULL);
}

/* format super block */
static 
void format ( MEM_SUPER_BLOCK *sb ) 
{
    printf ( "start to formate file system ...\n" );
    unsigned long base_sect,sects_nr;

    get_part_info (ROOT_DEV,&base_sect,&sects_nr);

    sb->sb_ninodes = 0xFFFF ;           // 65535 inodes 
    sb->sb_nzones  = (sects_nr >> 1);   // sect_nr/2 
    sb->sb_zmap_blocks = ZMAP_SLOTS;
    sb->sb_imap_blocks = IMAP_SLOTS;
    sb->sb_first_datazone = 1 + 1 + IMAP_SLOTS + ZMAP_SLOTS + ((IMAP_SLOTS * 8192 *  32)>>10) ; 
    sb->sb_log_zone_size = 0;   // in minix ,it is 0 
    sb->sb_max_size = ZMAP_SLOTS * 8 * 1024 * 1024;
    sb->sb_magic = CASEY_MAGIC;

    char buf[1024] = {0};
    memcpy (buf,(void*)sb,sizeof(DEV_SUPER_BLOCK));
    ll_rw_blk (WRITE,ROOT_DEV,1,1,buf);   // write back now 

    // get imap && zmap 
    BUFFER_HEAD *tmp;
    unsigned long i = 0 , blk_nr = 2;

    for ( i = 0 ; i < IMAP_SLOTS ; i ++ ,blk_nr ++) 
    {
        if ( !(tmp = get_blk (ROOT_DEV,blk_nr)) )    // imap 
            crash ( "can not read blk for format !\n" );

        memset (tmp->bh_buf,0,1024);
        tmp->bh_dirt = true;
        tmp->bh_valid = true;

        sb->sb_imap_bh_ptr[i] = tmp; 
    }
    for ( i = 0 ; i < ZMAP_SLOTS ; i ++ ,blk_nr ++)
    {
        if ( !(tmp = get_blk (ROOT_DEV,blk_nr)))   // zmap 
            crash ( "can not read blk for format !\n" );
            
        memset (tmp->bh_buf,0,1024);

        tmp->bh_dirt = true;
        tmp->bh_valid = true;

        sb->sb_zmap_bh_ptr[i] = tmp;
    }

    // set imap inode 0 used state 
    sb->sb_imap_bh_ptr[0]->bh_buf[0] = 0x01; // inode 0 is not used 
    sb->sb_imap_bh_ptr[0]->bh_dirt = true;

    sb->sb_dirt = true;
    printf ( "fs formated done !\n" );
}

static int called = false;

char load_root ( void ) 
{
    // load basic root file system 
    if ( !called )
    { 
        int i = 0 ,blk_nr = 2;

        // init super block array 
        memset (super_blk,0,sizeof(super_blk));

        MEM_SUPER_BLOCK *sb = read_sblk (ROOT_DEV);
        sb->sb_dev = ROOT_DEV;

        if ( sb->sb_magic != CASEY_MAGIC ) // casey have not installed 
        {
            printf ( "start casey installations ...\n" );
            format (sb);

            MEM_INODE *m_root ,*m_root_dir;
            DIR_ENTRY *bh_de_ptr;
            BUFFER_HEAD *bh;

            printf ( "making system directories ...  " );
            // new a inode for root 
            if ( !(m_root = new_inode (ROOT_DEV)) )   // inode == 1 (ROOT_INODE) 
                crash ( "can not create root inode!\n" );
            //new a inode for root dir inode  ,this inode nr must be 1 
            if ( !(m_root_dir = new_inode (ROOT_DEV)) ) 
                crash ( "can not create root dir inode !\n" );
            // new block for dir entry 
            if ( !(m_root->i_zone[0] = new_block (m_root->i_dev)) ) 
                crash ( "can not new block for root inode !\n" );

            m_root->i_size  = BLK_SIZE;
            m_root->i_mtime = m_root->i_acc_time = get_tks ();
            m_root->i_mode  = I_DIRECTORY;
            m_root->i_uid   = 0x1234;
            m_root->i_dirt  = true;
            m_root->i_valid = true;

            if ( !(bh = bread (m_root->i_dev,m_root->i_zone[0])) ) 
                crash ( "can not read root entries !\n" );

            bh_de_ptr = (DIR_ENTRY*)(bh->bh_buf);

            bh_de_ptr[0].inode_nr   = m_root_dir->i_num;
            bh_de_ptr[0].name[0]    = '/';
            bh_de_ptr[0].name[1]    = '\0';

            bh->bh_dirt = true;
            brelse (bh);

            // start to make root dir inode 
            if ( !(m_root_dir->i_zone[0] = new_block (m_root->i_dev)) )
                crash ( "can not new block for root dir inode !\n" );
            if ( !(bh = bread (m_root_dir->i_dev,m_root_dir->i_zone[0])) ) 
                crash ( "can not read root dir entries !\n" );
            m_root_dir->i_size  = BLK_SIZE;
            m_root_dir->i_mtime = m_root_dir->i_acc_time = get_tks ();
            m_root_dir->i_mode  = I_DIRECTORY;
            m_root_dir->i_uid   = 0x5678;
            m_root_dir->i_dirt  = true;
            m_root_dir->i_valid = true;

            bh_de_ptr = (DIR_ENTRY*)(bh->bh_buf);

            bh_de_ptr[0].inode_nr   = m_root_dir->i_num;  // current inr 
            bh_de_ptr[0].name[0]    = '.';
            bh_de_ptr[0].name[1]    = '\0';

            bh_de_ptr[1].inode_nr   = m_root->i_num;
            bh_de_ptr[1].name[0]    = '.';
            bh_de_ptr[1].name[1]    = '.';
            bh_de_ptr[1].name[2]    = '\0';

            bh->bh_dirt = true;
            brelse (bh);
            
            mkdir ("/etc",0);   
            mkdir ("/bin",0);
            mkdir ("/usr",0);   
            mkdir ("/tmp",0);
            mkdir ("/dev",0);   
            mkdir ("/var",0);
            mkdir ("/mnt",0);

            sync_bmaps (ROOT_DEV);
            sync_dev_inodes ();
            sync_blks (ROOT_DEV,-1);    // wb now !!!! 
            printf ( "\nmaking sys dirs compeleted ! \n" );

            /*
            printf ( "start to copying files to /bin " );

            int fd = 
            fd = do_open (1,"/bin/mm",O_CREAT,I_REGULAR);
            do_write (
            */
            printf ( "casey installed successfully !\n" );
        }
        else
        {
            for ( i = 0 ; i < IMAP_SLOTS ; i ++ ,blk_nr ++) 
                if ( !(sb->sb_imap_bh_ptr[i] = bread (ROOT_DEV,blk_nr)))   // imap 
                    crash ( "read imap block failed ! \n" );
            for ( i = 0 ; i < ZMAP_SLOTS ; i ++ ,blk_nr ++)
                if ( !(sb->sb_zmap_bh_ptr[i] = bread (ROOT_DEV,blk_nr)))   // zmap 
                    crash ( "read zmap block failed !\n" );
        }

        sb->sb_rd_only = false;
        sb->sb_time = get_tks ();
        sb->sb_locked = false;
        printf ( "total blocks %08X \n",sb->sb_nzones );

        called = true;

        return (0);
    }
    return (-1);
}
