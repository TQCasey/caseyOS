/* -----------------------------------------------------------------------------------------------------------
 * this is used for exceptions handlers 
 * 2012 - 3 - 14 
 * By Casey 
 * ----------------------------------------------------------------------------------------------------------*/

#ifndef __EXCPTN_H__
#define __EXCPTN_H__


extern void __do_except ( int vectno,		    //vect number 
	    	              int errcode,		    //error code
		    	          int eip,		        //eip
			              int ecs,		        //ecs
			              int eflags ); 		//eflag first push 
extern void __div_err (void);    
extern void __debug   (void);	        
extern void __nmi     (void);       
extern void __breakpt (void);       
extern void __overflow (void);   
extern void __bounds_chk (void);  	
extern void __inval_opcode (void); 
extern void __copr_unavailable (void); 
extern void __double_fault (void);
extern void __copr_seg_overrun (void);
extern void __inval_tss (void);
extern void __seg_unpresent	(void);   
extern void __stack_exception (void); 
extern void __general_protection (void);
extern void __page_fault ( unsigned long err_code );
extern void __copr_err (void);


#endif
