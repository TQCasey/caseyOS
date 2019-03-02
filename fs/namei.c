#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "fs.h"
#include <assert.h>
#include <casey/config.h>
#include <sys/stat.h>
#include <fcntl.h>

/* search from inode m_dir ,to find a dir_entry named 'entry_name' */
BUFFER_HEAD *find_entry ( MEM_INODE *m_dir,char *entry_name,
        unsigned long name_len,DIR_ENTRY **bh_de_ptr ) 
{
    if (NULL == m_dir||bh_de_ptr == NULL) return (NULL);
    if (!m_dir->i_dev) return (NULL);

    if (!name_len) {
        (*bh_de_ptr) = NULL;
        return (NULL);
    }
    int src_len;

    if (name_len > (NAME_LEN)) 
        name_len = NAME_LEN - 1;    /* truncate */

    BUFFER_HEAD *bh;
    long    i = 0,j = 0,blk_no,blk_nr = m_dir->i_size/BLK_SIZE + 
                          (0 != (m_dir->i_size%BLK_SIZE)); 
    DIR_ENTRY *de_ptr = NULL;

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
            src_len = strlen (de_ptr[j].name) ;
            name_len = (src_len > name_len) ? src_len : name_len;
            if ( !strncmp (de_ptr[j].name,entry_name,name_len) ) /*strncmp  */ 
            {
                (*bh_de_ptr) = &de_ptr[j];  /* bh_de_ptr is buffer's dir_entry ptr */
                return (bh);
            }
        }
        brelse (bh);
    }
    return (NULL);
}

/* add a entry to m_dir */
BUFFER_HEAD *add_entry ( MEM_INODE *m_dir,char *entry_name,
        unsigned long name_len,DIR_ENTRY **bh_de_ptr ) 
{
    if (NULL == m_dir||bh_de_ptr == NULL) return (NULL);
    if (!m_dir->i_dev) return (NULL);

    if (!name_len) {
        (*bh_de_ptr) = NULL;
        return (NULL);
    }

    unsigned char crted = false;
    if (name_len > NAME_LEN) 
        name_len = NAME_LEN - 1;    /* truncate */

    BUFFER_HEAD *bh;
    long    i = 0,j = 0,blk_no;
    DIR_ENTRY *de_ptr = NULL;

    for ( i = 0 ; i < (512 * 512 +512 + 7) ; i ++ ) 
    {   /* enumenate all blks */
        crted = false;
        if ( !(blk_no = create_block (m_dir,i,&crted)) ) return (NULL) ;

        if ( crted ) 
        {
            m_dir->i_size += BLK_SIZE;                 /* new block */
            m_dir->i_crt_time = get_tks ();
            m_dir->i_dirt = true;
        }

        if ( !(bh = bread (m_dir->i_dev,blk_no)) ) continue;    /* next one */

        de_ptr = (DIR_ENTRY*)(bh->bh_buf); /* start search in this buffer */

        for ( j = 0 ; j < ENTRIES_PER_BLK ; j ++  ) 
        {
            if (!de_ptr[j].inode_nr && !de_ptr[j].name[0])
            {
                m_dir->i_mtime = get_tks ();
                m_dir->i_dirt = true;
                strncpy (de_ptr[j].name,entry_name,name_len);
                de_ptr[j].name[name_len] = '\0';
                (*bh_de_ptr) = &de_ptr[j];
                bh->bh_dirt = true;
                return (bh);
            }
        }
        brelse (bh);
    }
    return (NULL);
}

/* get the last dir inode */
static 
MEM_INODE *get_last_dir ( const char *path )
{
    if ( !path ) return (NULL);

    MEM_INODE *m_dir;
    char *p = (char*)path,c ;
    BUFFER_HEAD *bh;
    DIR_ENTRY *bh_de_ptr ;
    unsigned short dev_no ;
    unsigned long inode_nr,name_len = 0;

    /*search from root inode */
    if (path[0] == '/' ){
        m_dir = iget (ROOT_DEV,ROOT_IND_NR);
        path ++ ;
    }
    else {
        printf ( "error : path is invalidate !\n" );
        return (NULL);
    }

    /* start to de-path */
    while ( true ) 
    {
        p = (char*)path;

        if ( !S_ISDIR(m_dir->i_mode) )  /* must be dir mode */
        {
            iput (m_dir);
            return (NULL);
        }
        /* get name len from path */
        for ( name_len = 0 ; (c = *path ++ ) && (c != '/') ; name_len ++ ) ;
        if ( !c ) {
            return (m_dir);
        }
        if ( !(bh = find_entry (m_dir,p,name_len,&bh_de_ptr)) ) /* can not find */
        {
            iput (m_dir);
            return (NULL);
        }
        
        inode_nr = bh_de_ptr->inode_nr;
        dev_no = m_dir->i_dev;
        MEM_INODE *m_inode = m_dir;

        iput (m_dir);
        brelse (bh);

        if ( !(m_dir = iget (dev_no,inode_nr)) )/* inode nr */
        {
            return (NULL);
        }
    }
    return (NULL);
}  

static 
MEM_INODE *dir_namei (const char *path,
        unsigned long *name_len,const char **name ) 
{
    MEM_INODE *m_dir;
    const char *base_name;
    char ch;

    if ( !(m_dir = get_last_dir (path)) )
        return (NULL);
    base_name = path;
    while ( (ch = *path ++) ) if ( '/' == ch ) base_name = path;
    *name_len = path - base_name - 1;    /* sub a '/' */
    *name = base_name ;
    return (m_dir);
}

/* namei () */
MEM_INODE *namei ( const char *path ) 
{
    const char *base;
    unsigned short dev_no;
    unsigned  long inode_nr,name_len = 0;

    DIR_ENTRY *bh_de_ptr;
    BUFFER_HEAD *bh;
    MEM_INODE *m_dir;

    if ( !(m_dir = dir_namei (path,&name_len,&base)) ) 
        return (NULL);
    if ( !name_len ) 
        return (m_dir);
    if ( !(bh = find_entry (m_dir,(char*)base,name_len,&bh_de_ptr)) ) 
        return (NULL);
    inode_nr = bh_de_ptr->inode_nr;
    dev_no = m_dir->i_dev;
    brelse (bh);
    iput (m_dir);

    if (m_dir = iget (dev_no,inode_nr) )
    {
        m_dir->i_acc_time = get_tks ();
        m_dir->i_dirt = true;
    }
    return (m_dir);
}

/* open inode */
int open_namei ( const char *path,unsigned short flag,
        unsigned short mode,MEM_INODE **dest_inode )
{
    const char *base ;
    unsigned long inode_nr = 0,name_len;
    unsigned short dev_no;
    MEM_INODE *m_prev_dir,*m_new_inode ;
    BUFFER_HEAD *bh;
    DIR_ENTRY *bh_de_ptr;

    /* read only && truncate ==> rw */
    if ( (flag & O_RDONLY) && !(flag&O_ACCMODE) )   flag |= O_WRONLY;

    //mode |= I_REGULAR;  /* regular file type */

    /* get prev dir inode */
    if ( !(m_prev_dir = dir_namei (path,&name_len,&base)) ) {
        printf ( "error : specified prev dir inode not found !\n" );
        return (-1);
    }

    /* no name specified ! */
    if ( !name_len ) {  /* open_namei () not hanhle mkdir ^_^ */
        if ( !(flag & (O_ACCMODE|O_CREAT|O_TRUNC)) ){   
            (*dest_inode) = m_prev_dir;
            return (0);
        }

        iput (m_prev_dir);
        printf ( "error : no file name specified !\n" );
        return (-1);
    }

    /* find this entry (base,name_len) */
    if ( !(bh = find_entry (m_prev_dir,(char*)base,name_len,&bh_de_ptr)) ) {
        if ( !(flag & O_CREAT) ) {    /* not create file */
            iput (m_prev_dir);
            printf ( "error : no entry specified found !\n" );
            return (-1);
        }

        /* create file inode */
        if ( !(m_new_inode = new_inode (m_prev_dir->i_dev)) ) {
            iput (m_prev_dir);
            printf ( "error : no more blk !\n" );
            return (-1);
        }

        m_new_inode->i_uid = 0xEEEE;
        m_new_inode->i_mode = mode;
        m_new_inode->i_dirt = true;
        m_new_inode->i_valid = true;

        /* add entry failed */
        if ( !(bh = add_entry (m_prev_dir,(char*)base,name_len,&bh_de_ptr)) ) {
            m_new_inode->i_nlinks = 0;
            iput (m_new_inode);
            iput (m_prev_dir);
            printf ( "error : no more blks !\n" );
            return (-1);
        }

        bh_de_ptr->inode_nr = m_new_inode->i_num;
        bh->bh_dirt = true; /* dirt */

        brelse (bh);
        iput (m_prev_dir);
        (*dest_inode) = m_new_inode;    /* return new file inode */
        return (0);
    }

    /* found one ,then open it */
    inode_nr = bh_de_ptr->inode_nr;
    dev_no = m_prev_dir->i_dev;

    /* free this rsc */
    iput (m_prev_dir);
    brelse (bh);    /* not dirt */

    /* ???? */
    if ( flag & O_EXCL ) return (-1);

    if ( !(m_new_inode = iget (dev_no,inode_nr)) )
        return (-1);

    /* if this inode is dir mode */
    if ( S_ISDIR (m_new_inode->i_mode) && (flag & O_ACCMODE) ){ /* denied or is dir mode */
        iput (m_new_inode);
        printf ( "error : this is dir inode !\n" );
        return (-1);
    }

    m_new_inode->i_acc_time = get_tks ();

    if ( flag & O_TRUNC )   /* truncate it */
        clear_iblks (m_new_inode);
    *dest_inode = m_new_inode ;

    return (0);
}

/* make a dir */
long mkdir ( const char *path ,unsigned short mode) 
{
    const char *base;
    unsigned long namelen ;
    MEM_INODE *m_prev_dir,*m_new_dir;
    BUFFER_HEAD *bh,*new_dir_block ;
    DIR_ENTRY *bh_de_ptr;

    if ( !(m_prev_dir = dir_namei (path,&namelen,&base)) ){
        printf ( "error : get parent dir inode failed !\n" );
        return (-1);
    }

    if ( !namelen ) {
        iput (m_prev_dir);
        printf ( "error : no dir name specified !\n" );
        return (-1);
    }

    /* already exist */
    if (bh = find_entry (m_prev_dir,(char*)base,namelen,&bh_de_ptr)) {
        brelse (bh);
        iput (m_prev_dir);
        printf ( "error : path is already exist !\n" );
        return (-1);
    }

    if (!(m_new_dir = new_inode (m_prev_dir->i_dev))) {
        iput (m_prev_dir);
        printf ( "error : no more inode is free !\n" );
        return (-1);
    }

    /* start to new inode ,use the same way like linux 0.11 */
    if ( !(m_new_dir->i_zone[0] = new_block (m_new_dir->i_dev)) ) {
        iput (m_prev_dir);
        m_new_dir->i_nlinks = 0;
        iput (m_new_dir);
        printf ( "error : no more block is free !\n" );
        return (-1);
    }

    m_new_dir->i_size = BLK_SIZE;
    m_new_dir->i_mtime = m_new_dir->i_acc_time = get_tks ();
    m_new_dir->i_dirt = true;

    if ( !(new_dir_block = bread (m_new_dir->i_dev,m_new_dir->i_zone[0])) ) {
        iput (m_prev_dir);
        free_block (m_new_dir->i_dev,m_new_dir->i_zone[0]);
        m_new_dir->i_nlinks = 0 ;
        iput (m_new_dir);
        printf ( "error : read block failed !\n" );
        return (-1);
    }

    bh_de_ptr = (DIR_ENTRY*)(new_dir_block->bh_buf);

    bh_de_ptr[0].inode_nr = m_new_dir->i_num  ;//current inode nr 
    bh_de_ptr[0].name[0] = '.';
    bh_de_ptr[0].name[1] = '\0';            /* "." */

    bh_de_ptr[1].inode_nr = m_prev_dir->i_num    ;//prev inode nr 
    bh_de_ptr[1].name[0] = '.';
    bh_de_ptr[1].name[1] = '.';
    bh_de_ptr[1].name[2] = '\0';

    m_new_dir->i_nlinks = 2;  /* name links == 2,1 for '.' 1 for self name dir */

    new_dir_block->bh_dirt = true;
    brelse (new_dir_block);

    m_new_dir->i_mode = I_DIRECTORY;
    m_new_dir->i_uid = 0xFFFF;
    m_new_dir->i_dirt = true;
    m_new_dir->i_valid = true;

    /* now every thing is ok ,start to add a entry at 
     * prev dir blocks 
     */

    if ( !(bh = add_entry (m_prev_dir,(char*)base,namelen,&bh_de_ptr)) ) {
        iput (m_prev_dir);
        free_block (m_new_dir->i_dev,m_new_dir->i_zone[0]);
        m_new_dir->i_nlinks = 0;
        iput (m_new_dir);
        printf ( "error : no more block is free for added entry !\n" );
        return (-1);
    }
    bh_de_ptr->inode_nr = m_new_dir->i_num;
    bh->bh_dirt = true;

    //printf ( "new dir inode nr %d \n",m_new_dir->i_num );
    m_prev_dir->i_nlinks ++;    /* add one for new dir */
    m_prev_dir->i_dirt = true;
    m_prev_dir->i_valid = true;
    iput (m_prev_dir);
    iput (m_new_dir);
    brelse (bh);
    return (0);
}

/* is dir empty ? @return 0 == is not empty  1 == is empty 
 * */
static 
unsigned char is_dir_empty ( MEM_INODE *m_dir )
{
    if (!m_dir || !m_dir->i_dev ) return (false);
    if (m_dir->i_size > BLK_SIZE) return (false);   /* but'.' && '..' take one block */ 

    unsigned short empty_cnt = 0;

    BUFFER_HEAD *bh;
    long  j = 0,blk_no; 
    DIR_ENTRY *de_ptr = NULL;

    /* to prove this m_dir is empty ,
     * just find a dir_entry which is not zero */

    if ( !(blk_no = bmap (m_dir,0)) )
        crash ( "dir is not correct !\n" ); /* next block */

    bh = bread (m_dir->i_dev,blk_no);
    if ( !bh ) return (false);

    empty_cnt = 0;
    de_ptr = (DIR_ENTRY*)(bh->bh_buf);      /* start search in this buffer */
    for ( j = 2 ; j < ENTRIES_PER_BLK ; j ++ ) 
    {
        if ( de_ptr[j].inode_nr && de_ptr[j].name[0]) 
            return (false);
    }
    brelse (bh);
    return (true);
}

/* remove a directory */
long rmdir ( const char *path ) 
{
    MEM_INODE *m_prev_dir,*m_this_dir;
    unsigned long name_len = 0;
    const char *base ;
    BUFFER_HEAD *bh;
    DIR_ENTRY *bh_de_ptr;

    /* invalidate path */
    if ( !(m_prev_dir = dir_namei (path,&name_len,&base)) ) {
        printf ( "error : trying to remove non-existent path dir !\n" );
        return (-1);
    }

    /* no path */
    if ( !name_len ) {
        printf ( "error : no name specified !\n" );
        iput (m_prev_dir);
        return (-1);
    }

    /* path dir_entry not found */
    if ( ! (bh = find_entry (m_prev_dir,(char*)base,name_len,&bh_de_ptr)) ) {
        printf ( "error : path dir entry not found !\n" );
        iput (m_prev_dir);
        return (-1);
    }

    /* ????? */
    if (!( m_this_dir = iget (m_prev_dir->i_dev,bh_de_ptr->inode_nr))){
        printf ( "error : get inode failed !\n" );
        iput (m_prev_dir);
        brelse (bh);
        return (-1);
    }

    /* dev && cnt chking ... ??? */
    if ( m_this_dir->i_dev != m_prev_dir->i_dev || m_this_dir->i_cnt > 1 ) {
        printf ( "error : device nr or count  is not correct now ! %x != %x ,cnt %d\n"
                ,m_this_dir->i_dev,m_prev_dir->i_dev,m_this_dir->i_cnt) ; 
        iput (m_prev_dir);
        iput (m_this_dir);
        brelse (bh);
        return (-1);
    }
    /* self rm */
    if ( m_prev_dir == m_this_dir ) {
        printf ( "trying to rm current dir !\n" );
        iput (m_prev_dir);
        iput (m_this_dir);
        brelse (bh);
        return (-1);
    }
    if ( m_this_dir->i_num == ROOT_NR )
    {
        iput (m_prev_dir);
        iput (m_this_dir);
        brelse (bh);
        printf ( "error : trying to rm ROOT nr\n" ) ;
        return (-1);
    }
    /* not dir */
    if ( !S_ISDIR (m_this_dir->i_mode)) {
        iput (m_prev_dir);
        iput (m_this_dir);
        brelse (bh);
        printf ( "error : trying to rm non-dir dir inode !\n" ) ;
        return (-1);
    }

    /* not empty */
    if ( !is_dir_empty (m_this_dir) ) {
        iput (m_prev_dir);
        iput (m_this_dir);
        brelse (bh);
        printf ( "error : trying to rm non-empty dir !\n" );
        return (-1);
     }

     if ( m_this_dir->i_nlinks != 2 ) {
        printf ( "error : this rming dir links is %d \n",m_this_dir->i_nlinks ) ;
        iput (m_prev_dir);
        iput (m_this_dir);
        brelse (bh);
    }

    bh_de_ptr->inode_nr = 0;
    memset (bh_de_ptr->name,0,NAME_LEN);
    bh->bh_dirt = true;
    brelse (bh);

    m_this_dir->i_nlinks = 0;
    m_this_dir->i_dirt  = false;
    /* this can not be dirt 'cause it is to be removed 
     * no need to write back to disk */

    m_prev_dir->i_nlinks -- ;
    m_prev_dir->i_mtime = m_this_dir->i_acc_time = get_tks ();
    m_prev_dir->i_dirt = true;  /* this is dir inode need not to 
                                 * write back ,however prev parent 
                                 * dir inode need to do this 
                                 */

    iput (m_prev_dir);
    iput (m_this_dir);

    return (0);
} 

/* create a hard link  rename */
int link ( const char *old_name,const char *new_name ) 
{ 
    DIR_ENTRY *de;
    MEM_INODE *m_old_inode,*m_new_prev_dir ;
    const char *base;
    unsigned long namelen = 0;
    BUFFER_HEAD *bh;

    /* no dir entry found */
    if ( !(m_old_inode = namei (old_name)) ){
        printf ( "error : no entry found in namei () !\n" );
        return (-1);
    }

    /* is dir mode */
    if ( S_ISDIR (m_old_inode->i_mode) ) {/* dir mode */
        iput (m_old_inode);
        printf ( "this is a dir inode !\n" );
        return (-1);
    }

    /* get new name prev dir inode */
    if ( !(m_new_prev_dir = dir_namei (new_name,&namelen,&base)) )
    { /* get new name prev dir inode */
        iput (m_old_inode);
        printf ( "error : old name inode access failed !\n" );
        return (-1);
    }

    /* no name specified !*/
    if ( !namelen ) {
        iput (m_old_inode);
        iput (m_new_prev_dir);
        printf ( "error : no name specified !\n" );
        return (-1);
    }
    /* dev_no is correspond  ?? */
    if ( m_old_inode->i_dev != m_new_prev_dir->i_dev ) {
        iput (m_old_inode);
        iput (m_new_prev_dir);
        printf ( "error : device nr not correspond ! %x != %x \ni",m_old_inode->i_dev,m_new_prev_dir->i_dev );
        return (-1);
    }

    /* chk the path if it is exist */
    if ( bh = find_entry (m_new_prev_dir,(char*)base,namelen,&de) ) {
        brelse (bh);
        iput (m_old_inode);
        iput (m_new_prev_dir);
        printf ( "error : new path entry already in !\n" );
        return (-1);
    }
    /* if not in ,add in */ 
    if ( !(bh = add_entry (m_new_prev_dir,(char*)base,namelen,&de)) ) {
        iput (m_old_inode);
        iput (m_new_prev_dir);
        printf ( "error : old inode has no more blk !\n" );
        return (-1);
    }

    de->inode_nr = m_old_inode->i_num;  /* */
    bh->bh_dirt = true;
    brelse (bh);
    iput (m_new_prev_dir);

    /* old name inode links ++ */
    m_old_inode->i_nlinks ++;
    m_old_inode->i_acc_time = get_tks ();
    m_old_inode->i_dirt = true;

    iput (m_old_inode);
    return (0);
}

/* rm a hard link for inode */
int unlink ( const char *name ) 
{
    const char *base;
    unsigned long namelen ;
    MEM_INODE *m_prev_dir,*m_this_inode;
    BUFFER_HEAD *bh;
    DIR_ENTRY *bh_de_ptr;

    /* prev dir inode */
    if ( !(m_prev_dir = dir_namei (name,&namelen,&base)) ){
        printf ( "error : prev dir inode not found !\n" );
        return (-1);
    }

    /* no name specified */
    if ( !namelen ) {
        iput (m_prev_dir);
        printf ( "error : no name specified !\n" );
        return (-1);
    }

    /* find in m_prev_inode */
    if ( !(bh = find_entry (m_prev_dir,(char*)base,namelen,&bh_de_ptr)) ) {
        iput (m_prev_dir);
        printf ( "error : entry not found !\n" );
        return (-1);
    }

    if ( !(m_this_inode = iget (m_prev_dir->i_dev,bh_de_ptr->inode_nr)) ) {
        iput (m_prev_dir);
        brelse (bh);
        return (-1);
    }

    /* this dir inode is dir mode */
    if ( S_ISDIR (m_this_inode->i_mode) ) { 
        iput (m_this_inode);
        iput (m_prev_dir);
        brelse (bh);
        printf ( "specified name inode is dir !\n" );
        return (-1);
    }

    if ( !m_this_inode->i_nlinks ) {
        printf ( "deleting non-existent file (dev 0x%x,i_nr %d) with links  = 0 !\n",
                m_this_inode->i_dev,m_this_inode->i_num );
        m_this_inode->i_nlinks = 1;
    }

    bh_de_ptr->inode_nr = 0;
    memset (bh_de_ptr->name,0,NAME_LEN);
    bh->bh_dirt = true;
    brelse (bh);

    m_this_inode->i_nlinks --;
    m_this_inode->i_dirt = true;
    m_this_inode->i_mtime  = get_tks ();
    iput (m_this_inode);
    iput (m_prev_dir);
    return (0);
}
