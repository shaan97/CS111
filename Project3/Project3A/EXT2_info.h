#ifndef EXT2_INFO_H
#define EXT2_INFO_H

#include "Ext2_fs.h"

// Keep track of some information between function calls
struct EXT2_info{
  ext2_super_block super_block;
  ext2_group_desc des_table;
};


#endif
