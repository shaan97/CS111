//NAME: Shaan Mathur, Garvit Pugalia
//EMAIL: shaankaranmathur@gmail.com, garvitpugalia@gmail.com
//ID: 904606576, 504628127

#ifndef EXT2_INFO_H
#define EXT2_INFO_H

#include <linux/types.h>
#include "Ext2_fs.h"

// Keep track of some information between function calls
class EXT2_info{
 private:
  char * d_bmap;
  char * i_bmap;
 public:
  ext2_super_block* super_block;
  ext2_group_desc* des_table;
  int image_fd;

  EXT2_info();
  ~EXT2_info();
  void init_b_maps();
  bool is_valid_block(__u32 block) const;
  bool is_valid_inode(__u32 inode) const;
};


#endif
