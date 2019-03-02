#include <casey/types.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <casey/hdcntl.h>
#include "fs.h"
#include <casey/config.h>
#include <stdarg.h>
#include <string.h>


#define HZ  100

static HDINFO s_hdinfo[MAX_DRVS] ;

/* this is the decl for kernel module */
extern void   *rdports(short port,const char *dest,unsigned long  size);
extern unsigned long  wrports(short port,const char *src ,unsigned long  size);
extern unsigned char wrport ( unsigned short port,unsigned char val );

inline unsigned char MAKE_HDREG (unsigned char lba,
            unsigned char drv,unsigned char highlba ) 
{
    return ( (lba << 6) | (drv << 4)
           | (highlba & 0x0F) | 0xA0 ) ;
} 


/* read just one byte from port */
unsigned char   rdport  ( unsigned short port )
{
    __byte tmp = 0x00;
    rdports (port,(const void*)&tmp,1);
    return (tmp);
}

/* write just one byte to port */
unsigned char   wrport  (unsigned short port,unsigned char val)
{
    return ( *((__byte*)wrports (port,(const void*)&val,1)) );
}

static 
unsigned char cntler_rdy( unsigned char  mask,
                          unsigned char status,
                          unsigned long ms_timeout ) 
{ 
    unsigned long  t = get_tks ();

    while ( ((get_tks () - t) * 1000 / HZ ) < ms_timeout )
        if ( (rdport (REG_STATUS) & mask ) == status ) return (1);  /* ok ! */
    return ( rdport (REG_ERR)); /* get err code */
} 

static void set_hd_cmd ( HDCMD *pcmd ) 
{
    /* write cmd to the cmd reg without the status busy is  STATUS_NO is not allowed ! */
   if ( !cntler_rdy (STATUS_BSY,STATUS_NO,STATUS_TIMEOUT) ) 
       crash ( "hard disk driver not ready!!!\n" ) ;

   wrport (REG_DEV_CTRL,0             );
   wrport (REG_FEATRUES,pcmd->features);
   wrport (REG_NSECTS  ,pcmd->csects  );
   wrport (REG_LBA_LOW ,pcmd->LBAlow  );
   wrport (REG_LBA_MID ,pcmd->LBAmid  );
   wrport (REG_LBA_HIGH,pcmd->LBAhigh );
   wrport (REG_DEV     ,pcmd->dev     );
   wrport (REG_CMD     ,pcmd->cmd     );
}

/* return out_bytes */
unsigned long ll_rw_phys_sect (unsigned char rw_rq,                  /* request r/w */
                               unsigned char drv_no,                 /* hard disk nr */
                               unsigned long sect_no,                /* from sect nr */
                               unsigned long sects_nr,               /* sects rw in cnt */
                               void*         dest )                  /* buffer ptr */
{ 
    HDCMD hdcmd ;

    if ( !dest || !sects_nr || sects_nr > 0xFF) return (0);        /* failed */

    hdcmd.features = 0;
    hdcmd.csects   = sects_nr & 0xFF;    /* how many sects to rw ?*/
    hdcmd.LBAlow   = sect_no & 0xFF;
    hdcmd.LBAmid   = (unsigned char)((sect_no >> 8) & 0xFF );
    hdcmd.LBAhigh  = (unsigned char)((sect_no >>16) & 0xFF );
    /* LBA Mode */
    hdcmd.dev      = MAKE_HDREG(1,drv_no,(sect_no >> 24) & 0x0F ) ;/* hard disk 0 */
    hdcmd.cmd      = (rw_rq == READ) ? ATA_READ : ATA_WRITE ;

    /* wait for intr wake up */
    set_hd_cmd (&hdcmd);

    /* nop some while */
    delay_ms (10);  

    /* ok let's do our jobs */
    if (rw_rq == READ)
    {
        wait_hd_intr ();
        rdports (REG_DATA,dest,(sects_nr<<9));
        return (sects_nr);
    }
    unsigned long nr = wrports(REG_DATA,dest,(sects_nr<<9));
    wait_hd_intr ();
    return (nr);
}


unsigned long ll_rw_blk ( unsigned char  rw_rq,                 /* request r/w */
                          unsigned short dev_no,                /* dev no */
                          unsigned long  blk_no,                /* block nr */
                          unsigned long  blks_nr,               /* blocks cnt */
                          void *         dest )
{
    /* get drv no */
    unsigned char major = MAJOR(dev_no),minor = MINOR(dev_no);
    if ( major > MAX_DEVS ) 
    {
        printf ( "error : Illegal driver number: 0x%x !\n",major );
        return (0);
    }
    if ( DEV_HD == major ) /* hd */
    {
        unsigned long start_sect = 0 ;
        if ( minor < 5 ) /* primary partition */
        {
            minor -- ;
            if ( (char)minor < 0 )
            {
                printf ( "error : Illegal dev number  0x%x !\n",minor );
                return (0);
            }
            start_sect = s_hdinfo[0].prim_part_table[minor].start_phys_sect_no;
            blks_nr <<= 1;

            if ( blks_nr > s_hdinfo[0].prim_part_table[minor].sects )
            {
                printf ( "sects nr too big !\n" );/* current phys sect */
                return (0);
            }
            start_sect += (blk_no<<1);
            return (ll_rw_phys_sect (rw_rq,0,start_sect,blks_nr,dest));
        }

        /* logical partition */
        minor -= 5;
        if ( (char)minor < 0 )
        {
            printf ( "error : Illegal dev number  0x%x !\n",minor );
            return (0);
        }
        else 
        {
            start_sect = s_hdinfo[0].logic_part_table[minor].cur.start_phys_sect_no;/* current phys sect */
            blks_nr <<= 1;

            if ( blks_nr > s_hdinfo[0].logic_part_table[minor].cur.sects )
            {
                printf ( "sects nr too big !\n" );/* current phys sect */
                return (0);
            }
            start_sect += (blk_no << 1);
            return (ll_rw_phys_sect (rw_rq,0,start_sect,blks_nr,dest));
        }
    }
    printf ( "unknown device type !\n" );
    return (0);
}


/* free buffer blk */
void brelse ( BUFFER_HEAD *bh_free )
{
    if ( !bh_free ) return ;
    if  ( !(bh_free->bh_cnt -- ) ) 
        crash ( "brelse : trying to free free buffer !\n" );
    if ( !bh_free->bh_cnt )     /* if count = 0 && dirty && validate,add to dirt_list */
    {
        if ( !bh_free->bh_dirt || !bh_free->bh_valid )
            free_buffer (bh_free);  /* if no need to wb ,free hash */
    }
}

/* get a blk buffer  */
BUFFER_HEAD *get_blk ( unsigned short dev_no,unsigned long blk_no ) 
{
    BUFFER_HEAD *tmp = NULL;
    /* get buffer */
    if ( !(tmp = find_buffer (dev_no,blk_no)) )  /*find buffer in hash */
    {
        if  ( (tmp = get_buffer (dev_no,blk_no)) )    /* not in hash ,get one from free_list */ 
        {
            tmp->bh_locked = false; /* not locked */
            tmp->bh_cnt ++;         
            tmp->bh_dev = dev_no;
            tmp->bh_valid = false; 
            tmp->bh_dirt = false;   /* not validate && not dirt */
            tmp->bh_blk_nr = blk_no;
            return (tmp);
        }
        else    /* this is not might happened while the code is ok !*/
            crash ( "no more block buffer free !\n" );
    }
    tmp->bh_cnt ++; /* if in hash */
    return (tmp);
}

/* read blk */
BUFFER_HEAD *bread ( unsigned short dev_no,unsigned long blk_no ) 
{
    BUFFER_HEAD *tmp = get_blk (dev_no,blk_no);
    if ( NULL == tmp ) 
        crash ( "panic : no more free buffer !\n" );
    if ( tmp->bh_valid ) return (tmp);  /* if in hash ,return tmp */

    ll_rw_blk (READ,dev_no,blk_no,1,tmp->bh_buf);
    tmp->bh_valid = true;   /* READ not dirt && valid */

    return (tmp);
} 

/* read a page bytes to specified address */
void* bread_page ( unsigned long addr,unsigned short dev_no,unsigned long blk_no[4] ) 
{
    BUFFER_HEAD *tmp ;
    int i = 0;

    for ( i = 0 ; i < 4 ; i ++ ) 
    {
        tmp = bread (dev_no,blk_no[i]);
        memcpy ((void*)addr,(const void*)tmp->bh_buf,1024);
        addr += BLK_SIZE;   /*next buffer */
        brelse (tmp);   /*not used again */
    }
    /* done ! */
    return ((void*)addr);
}

/* read some more blks from dev return the first blk_buffer head */
BUFFER_HEAD* breada ( unsigned short dev_no,unsigned long blk_no ,...) 
{
    va_list va_p ;
    BUFFER_HEAD *bh,*tmp;

    va_start (va_p,blk_no);

    if ( !(bh = bread (dev_no,blk_no)) ) 
        crash ( "breada (): get block return NULL !\n" );
    while ( (blk_no = va_arg(va_p,int)) >= 0 ) 
    {
        if ( tmp = bread (dev_no,blk_no) ) 
            if ( tmp->bh_valid ) 
                tmp->bh_cnt --;
    }
    return (bh);
}
/* .... */
static 
void get_part_table ( unsigned char drv_no,unsigned long mbr_start_sect ,int logic_no)
{
    char buf[512] = {0};

    ll_rw_phys_sect (READ,drv_no,mbr_start_sect,1,buf);   /* get MBR */

    if ( !mbr_start_sect )  /* primary partition table */
        memcpy (s_hdinfo[drv_no].prim_part_table,buf+PART_TABLE_OFF,64); /* 16B * 4 */ 
    else                    /* logical partition table */ 
        /* mbr_start_sect  == base */
        memcpy (&(s_hdinfo[drv_no].logic_part_table[logic_no]),buf+PART_TABLE_OFF,64);
}

/* show the hard disk part table */
static 
unsigned long disp_part_tbl ( unsigned char drv_no ) 
{
    int ext = 0;
    unsigned long logic_base_phys_sect_no = 0,logic_size = 0,minor = 1;

    get_part_table (drv_no,0,-1);  
    /* show primary DPTs */
    printf ( "dev_name===start_sect===setctors \n" );
    for ( ext = 0 ; ext < MAX_PRIMS ; ext ++ ) 
    {
        if ( !s_hdinfo[drv_no].prim_part_table[ext].sects ) break;
        printf ( "/dev/sda%d  0x%08X  0x%08X\n",
                minor++,
                logic_base_phys_sect_no = s_hdinfo[drv_no].prim_part_table[ext].start_phys_sect_no,
                logic_size = s_hdinfo[drv_no].prim_part_table[ext].sects );
    }

    if ( logic_size )   /*if there is logical partition tables */
    {
        unsigned long next_part_size = 0, /* next partition size */
                      next_start_sect = 0;/* next start sect no */
        ext = 0;
        minor = 5;
        /* show logical disk partiton tables */
        do{
            get_part_table (drv_no,logic_base_phys_sect_no + next_start_sect,ext);/* get part table */ 

            s_hdinfo[drv_no].logic_part_table[ext].cur.start_phys_sect_no 
                += (logic_base_phys_sect_no +  next_start_sect);

            next_start_sect = s_hdinfo[drv_no].logic_part_table[ext].next.start_phys_sect_no;
            next_part_size  = s_hdinfo[drv_no].logic_part_table[ext].next.sects;
            printf ( "/dev/sda%d  0x%08X  0x%08X\n",
                minor ++ ,
                s_hdinfo[drv_no].logic_part_table[ext].cur.start_phys_sect_no,  
                /* this is logical base
                 * the real base is the prev next start_sect
                 * plus the current start_sect 
                 */
                s_hdinfo[drv_no].logic_part_table[ext].cur.sects );

            if ( ext ++ > MAX_LOGICS ) 
                crash ( "total logical partition tables 0x%8X\n",ext );

        }while ( next_part_size != 0 ); 

    }
    printf ( "partition tables ok ! \n" );
    return (0);
}

/* this is called just ONLY once */ 
static 
void hd_identify ( unsigned char drv_no ) 
{
    HDCMD hdcmd;

    hdcmd.cmd = ATA_IDENTIFY;
    hdcmd.dev = MAKE_HDREG (0,drv_no,0);

    set_hd_cmd (&hdcmd);

    wait_hd_intr ();                    /* wait for hd intr */

    short buf[256] = {0};
    rdports (REG_DATA,(void*)buf,512);         /* get the buffer content */

    s_hdinfo[0].all.base = 0;           /* entire hard disk */
    s_hdinfo[0].all.size = getdw (buf+60);
}

void hd_info_init ( void ) 
{
    hd_identify (0);        /* hard disk 1 */
    printf ( "total sectors : 0x%0X\n", s_hdinfo[0].all.size  ) ;
    disp_part_tbl (0);
}

unsigned char get_part_info ( unsigned short dev_no,unsigned long *base_sect,unsigned long *sects_nr ) 
{
    /* get drv no */
    unsigned char major = MAJOR(dev_no),minor = MINOR(dev_no);
    if ( major > MAX_DEVS ) 
    {
        printf ( "error : Illegal dev number !\n" );
        return (0);
    }
    if ( DEV_HD == major ) /* hd */
    {
        if ( minor < 5 ) /* primary partition */
        {
            minor -- ;
            if ( (char)minor < 0 )
            {
                printf ( "error : Illegal minor  number !\n" );
                return (0);
            }
            *base_sect = s_hdinfo[0].prim_part_table[minor].start_phys_sect_no;
            *sects_nr  = s_hdinfo[0].prim_part_table[minor].sects;
            return (0);
        }

        /* logical partition */
        minor -= 5;
        if ( (char)minor < 0 )
        {
            printf ( "error : Illegal minor  number !\n" );
            return (0);
        }
        else 
        {
            *base_sect = s_hdinfo[0].logic_part_table[minor].cur.start_phys_sect_no;/* current phys sect */
            *sects_nr = s_hdinfo[0].logic_part_table[minor].cur.sects;
            return (1);
        }
    }
    printf ( "unknown device type !\n" );
    return (0);
}
