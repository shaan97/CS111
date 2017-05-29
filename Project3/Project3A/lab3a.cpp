#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "lab3a.h"
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "EXT2_info.h"

using namespace std;


void Print(std::string message){
  static ofstream file;
  static bool init = false;
  if (init == false)
    {
      file.open("lab3a.csv", std::ofstream::trunc);
      init = true;
    }
  file << message;
}

ssize_t Pread(int fd, void *buf, size_t count, off_t offset)
{
  if (pread(fd,buf,count,offset) < 0)
    {
      fprintf(stderr, "Error Number:%d\tError Message:%s\n", errno, strerror(errno));
      exit(1);
    }
}

string get_file_name(int argc, char *argv[]){
  if(argc != 2){
    fprintf(stderr, "Illegal number of arguments.\n");
    exit(1);
  }

  return string(argv[1]);
}

int Open(const char * pathname, int flags){
  int rc;
  if ( (rc = open(pathname, flags)) < 0 ){
    cerr << "Error Number:" << errno << "\tError Message: " << strerror(errno) << endl;
  }
  return rc;
}

// Collects super block information at offset SUPERBLOCK_OFFSET (0x400)
void getSuperblock(EXT2_info& info){
  info.super_block = new ext2_super_block;
  ssize_t rc = Pread(info.image_fd, info.super_block, sizeof(ext2_super_block), SUPERBLOCK_OFFSET);
  stringstream ss;
  ss << "SUPERBLOCK," << info.super_block->s_blocks_count << "," << info.super_block->s_inodes_count << ","
     << (EXT2_MIN_BLOCK_SIZE << info.super_block->
}

// Collects group descirptor table information, using computed offset given in super block
void getGroupDescriptor(EXT2_info &info){
  info.des_table = new ext2_group_desc;
  off_t offset = SUPERBLOCK_OFFSET + (EXT2_MIN_BLOCK_SIZE << info.super_block->s_log_block_size);
  ssize_t rc = Pread(info.image_fd, info.des_table, sizeof(ext2_group_desc), offset);
}

int main(int argc, char *argv[])
{
  string file = get_file_name(argc, argv);
  EXT2_info info;
  info.image_fd = Open(file.c_str(), O_RDONLY);

  getSuperblock(info);
  getGroupDescriptor(info);
  
  return 0;
}
