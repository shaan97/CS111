//#include <stdint.h>
#include <sys/types.h>
#include "EXT2_info.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>

#define SUPERBLOCK_OFFSET 0x400

EXT2_info::EXT2_info() {
  super_block = nullptr;
  des_table = nullptr;
  d_bmap = nullptr;
}

EXT2_info::~EXT2_info(){
  if(super_block != nullptr)
    delete super_block;
  if(des_table != nullptr)
    delete des_table;
  if(d_bmap != nullptr)
    delete d_bmap;
}

ssize_t Pread(int fd, void *buf, size_t count, off_t offset);
/*{
  if (pread(fd,buf,count,offset) < 0)
    {
      fprintf(stderr, "Error Number:%d\tError Message:%s\n", errno, strerror(errno));
      exit(1);
    }
}
*/

void EXT2_info::init_b_map() {
  if(d_bmap == nullptr) {
    int b_size = EXT2_MIN_BLOCK_SIZE << super_block->s_log_block_size;
    d_bmap = new char[b_size];
    off_t offset = SUPERBLOCK_OFFSET + (des_table->bg_block_bitmap - 1) * b_size;
    ssize_t rc = Pread(image_fd, d_bmap, b_size, offset);
  }
}

// Returns nonzero if block is in use
bool EXT2_info::is_valid_block(__u32 block) const {
  return block && d_bmap[ block / (8 * sizeof(char)) ] & (0x1 << (block % (8 * sizeof(char))));
}
