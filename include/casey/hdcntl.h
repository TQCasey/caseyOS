/* ------------------------------------------------------------------
 * for IBM AT hard disk control head file
 * 2012 - 3 - 29 
 * By Casey
 * -----------------------------------------------------------------*/
#ifndef __HDCNTL_H__
#define __HDCNTL_H__

#include <casey/types.h>

/* hd reg cmd */
typedef struct tagHDCMD{
    __u8    features;
    __u8    csects;
    __u8    LBAlow;
    __u8    LBAmid;
    __u8    LBAhigh;
    __u8    dev;
    __u8    cmd;
}HDCMD,*LPHDCMD;


#define HD_NUM_ADDR                  0x475

/* dev reg */
#define REG_DATA                     0x1F0                  /* hard disk data port */
#define REG_FEATRUES                 0x1F1                  /* error bits */
#define REG_ERR                      0x1F1                  
#define REG_NSECTS                   0x1F2                  /* nr of sectors to r/w */
#define REG_LBA_LOW                  0x1F3                  
#define REG_LBA_MID                  0x1F4
#define REG_LBA_HIGH                 0x1F5
#define REG_DEV                      0x1F6
#define REG_CMD                      0x1F7
#define REG_STATUS                   0x1F7

/* status reg */
#define STATUS_BSY                   0x80
#define STATUS_DRVRDY                0x40
#define STATUS_DFSE                  0x20                   /* drv panic */
#define STATUS_DSC                   0x10
#define STATUS_DRQ                   0x08                   /* dev request */
#define STATUS_CORR                  0x04                   /* corrected error  ECC err */
#define STATUS_IDX                   0x02                   /* recved indec */
#define STATUS_ERR                   0x01                   /* exec err */

#define STATUS_YES                   true
#define STATUS_NO                    false
#define STATUS_TIMEOUT               10000                  /* 10 s timeout */
/* reg port */
#define REG_DEV_CTRL                0x3F6
#define REG_ALT_STATUS              REG_DEV_CTRL
#define REG_DRV                     0x3F7

/* cmd/notifications */
#define ATA_IDENTIFY                0xEC
#define ATA_READ                    0x20
#define ATA_WRITE                   0x30

/* partitions table offset */
#define PART_TABLE_OFF              0x1BE

/* a sector size == 512 bytes */
#define SECT_SIZE                   512

/* hard disk number defs*/  
#define MAX_DRVS                    1               /* max hard disk numbers */                                
#define MAX_PRIMS_PER_DRV           4               /* max disk partition tables per drv */
#define MAX_LOGICS_PER_EXT_PART     8               /* max primary logic partition tables per ext part */ 

#define MAX_PRIMS                   MAX_PRIMS_PER_DRV
#define MAX_LOGICS                  MAX_LOGICS_PER_EXT_PART

/* part / system id */
#define NOPART                      0x00
#define EXTENED                     0x05

inline unsigned char MAKE_HDREG (unsigned char lba,unsigned char drv,unsigned char highlba ); 

/* parts info argt */
typedef struct tasgPARTINFO{
    __u32      base;                    /* base sect_no */
    __u32      size;                    /* how many sects_nr */
}PARTINFO,*LPPARTINFO;

/* Disk Partition Table (16bytes) */
typedef struct tagPART_TABLE{
    __u8        boot_indicator;         /* boot sig */
    __u8        start_head_no;          /* start hear no */ 
    __u8        start_sect_no;          /* start sect no */
    __u8        start_cyl_no;           /* start clyinder no */
    __u8        system_id;              /* part system id */
    __u8        end_head_no;            /* end head no */
    __u8        end_sect_no;            /* end sect no */
    __u8        end_cyl_no;             /* end cylinder no */
    __u32       start_phys_sect_no;     /* start phys_sect_no */
    __u32       sects;
}PART_TABLE;    


/* logic part table */
typedef struct tagLOGIC_PART_TABLE{
    PART_TABLE  cur,next,unused[2];
}LOGIC_PART_TABLE;


/* main drv  (only one hard disk)*/
typedef struct tagHDINFO{
    long                cnt;
    PARTINFO            all;
    PART_TABLE          prim_part_table[MAX_PRIMS];
    LOGIC_PART_TABLE    logic_part_table[MAX_LOGICS];
}HDINFO,*LPHDINFO;


/* for hard disk io ctrl */
typedef struct tagHDIOCTL{
    __u32       request;
    void*       *src;
    void*       *dest;
    __u32       size;
}HDIOCTL,*LPHDIOCTL;

/* for hard disk read write opt */
typedef struct tagHDRW{
    __u64       pos;        /* from where (in bytes) */
    void*       dest;       /* to which buffer */
    __i64       cnt;        /* copy how many bytes */
}HDRW,*LPHDRW;

#endif
