#include "fs.h"
#include <casey/mm.h>

FILE_STRUCT file_table[MAX_OPEN_FILES];
PFILE_STRUCT filp_ptr[MAX_FILPS * MAX_OPEN_FILES];  
