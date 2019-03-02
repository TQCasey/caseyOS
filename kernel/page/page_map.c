#include <casey/types.h>
#include <casey/kernel.h>
#include <casey/sched.h>

u32   MAPED_MEM;                                  // mem for mapping ...
u32   NR_PAGES    = 0;                            // mem pages of MAPED_MEM
u8    *page_cnt   = NULL;                         // pages cnt of MAPED_MEM pages


/* fresh CR3 */
#define fresh_page_dir()    \
    __asm__ ("movl  %%eax,%%cr3"::"a"(0x100000))

/* find a page free ,return phys addr if ok,
 * orelse return null for bad situation 
 * this is run in kernel space !!!!!!!  */

u32 find_empty_page ( void ) 
{
    static u32 i = 0 ;
    for ( ; i < NR_PAGES ; i = ((i + 1) % NR_PAGES) ) 
    {
        if ( ! page_cnt[i] )                        // find the first free page
        {
            page_cnt[i] ++ ;
            memset ((void*)(LOW_MEM+4096 * i),0,0x1000);
            return (LOW_MEM+4096*i); 
        }
    }
    return (0);
}

/* free a physical  page */
void free_page ( u32 phys_addr ) 
{
    if ( phys_addr < LOW_MEM ) return ;
    if ( phys_addr >= __mem_size ) 
        panic ( "trying to free nonexistent page (phys addr 0x%0X) \n",phys_addr ) ;
    u32 page_index = (phys_addr - LOW_MEM)>>12;
    if ( !page_cnt[page_index] )
        panic ("trying to free a free page!\n" ) ;
    page_cnt[page_index] --;
}

/* free page_dir () to free blk_size pages 
 * just handles 4MB blk!!!!!!!!!!!!!!!!!!*/

void free_page_dir_tables ( u32 page_addr,u32 page_dir_nr ) 
{
    u32 *page_dir_table = (u32*)(((page_addr>>20)&0xFFC) + 0x100000),
          *page_table,i = 0 ,j = 0;
    if ( page_addr & 0x3FFFFF )// if it is not 4MB alignment
        panic ("free page dir called with wrong alignmnet!\n" );    
    for ( i = 0 ; i < page_dir_nr ; i ++,page_dir_table++ )
    {
        if ( !(1&(*page_dir_table)) ) continue;
        page_table = (u32*)(0xFFFFF000&(*page_dir_table));
        if ( page_table == (void*)0 ) 
            panic ( "page dir table already cleared !\n" );
        for ( j = 0; j < 1024 ; j ++ )
        {
            if ( 1&(page_table[j]) ) 
            {
                free_page (0xFFFFF000&page_table[j]);
                page_table[j] = 0 ;
            }
        }
        free_page (0xFFFFF000&(*page_dir_table));
        *page_dir_table = 0;
    }
    fresh_page_dir ();
}

/* only kernel can do it */
u32 map_page_to_phys ( u32 phys_addr,u32 page_addr ) 
{

    /* IF IT IS TASKS */
    if ( __curprocs->pid >= NR_TASKS ) 
    {
        if ( phys_addr < LOW_MEM || phys_addr >= __mem_size ) 
            panic ( "trying to map page addr to non-maped zone (0x%0x)\n",phys_addr );
        if ( page_cnt[(phys_addr - LOW_MEM)>>12] == 0 ) 
            panic ( "trying to map page addr to a free page\n" ) ;
    }

    /* now chking completed ...,start to map*/
    u32 *page_dir_table = (u32*)(((page_addr>>20)&0xFFC) + 0x100000),
          new_dir_obj,*page_table;

    /* if page_dir_index < max_page_dir_index ) */
    if ( *page_dir_table&1 ) 
        page_table = (u32*)(0xFFFFF000&(*page_dir_table));
    else
    {
        /* if page_dir_index > max_page_dir_index ,then get one */
        if (! (new_dir_obj = find_empty_page ()) ) 
            panic ("no free page Available!\n" );
        *page_dir_table = new_dir_obj | 7; 
        page_table = (u32*) new_dir_obj;
    }
    page_table [(page_addr>>12)&0x3FF] = phys_addr | 7;

    return (phys_addr);
}

void __do_no_page ( u32 page_addr  ) 
{
    map_page_to_phys (find_empty_page (),page_addr );
}

/* init mm module */
inline void init_mm ( void ) 
{
    MAPED_MEM   = __mem_size - LOW_MEM;
    NR_PAGES    = MAPED_MEM >> 12 ;
    page_cnt    = (u8*)(0x101000+(__mem_size>>10));
    /* page map init */
    int  i = 0 ;
    for ( i = 0 ; i < NR_PAGES ; i ++ ) 
        page_cnt[i] = 0 ;
}

/* for debug ONLY in kernel */
void dump_page_dirs ( void ) 
{
    u32 *page_dir_table = (u32*)(0x100000);
    u32 i = 0 ;
    while (i ++ < 1024 )
    {
        if (  *page_dir_table & 1 ) 
            printk ("page_dir[%d] is 0x%0x\n",i - 1,*page_dir_table);
        page_dir_table ++;
    }
}

/* dump_page_dir_page_tables 4MBytes */
void dump_page_dir_tables ( u32 page_addr  ) 
{
    u32 *page_dir_table = (u32*)(((page_addr>>20)&0xFFC) + 0x100000),*page_table,j = 0;
    if ( page_addr & 0x3FFFFF ) // if it is not 4MB alignment
        panic ("free page dir called with wrong alignmnet!\n" );    

    page_table = (u32*)(0xFFFFF000&(*page_dir_table));
    if ( page_table == (void*)0 ) return ;
    for ( j = 0; j < 1024 ; j ++ )
        if ( 1&(page_table[j]) ) 
            printk ( "page_table_page 0x%0x ",page_table[j]);
}
