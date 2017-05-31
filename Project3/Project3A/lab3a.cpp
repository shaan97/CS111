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
#include <sstream>
#include <ctime>
#include <iomanip>
#include <cmath>

using namespace std;


void Print(const std::string message){
  /*
  static ofstream file;
  static bool init = false;
  if (init == false)
    {
      file.open("lab3a.csv", std::ofstream::trunc);
      init = true;
    }
  */
  cout << message;
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

  // Build stringstream to gather data
  stringstream ss;
  ss << "SUPERBLOCK," << info.super_block->s_blocks_count << ","
     << info.super_block->s_inodes_count << ","
     << (EXT2_MIN_BLOCK_SIZE << info.super_block->s_log_block_size) << ","
     << info.super_block->s_inode_size << ","
     << info.super_block->s_blocks_per_group << ","
     << info.super_block->s_inodes_per_group << ","
     << info.super_block->s_first_ino << endl;

  Print(ss.str());  // Print data into file
}

// Collects group descirptor table information, using computed offset given in super block
void getGroupDescriptor(EXT2_info &info){
  info.des_table = new ext2_group_desc;
  off_t offset = SUPERBLOCK_OFFSET + (EXT2_MIN_BLOCK_SIZE << info.super_block->s_log_block_size);
  ssize_t rc = Pread(info.image_fd, info.des_table, sizeof(ext2_group_desc), offset);
  stringstream ss;
  ss << "GROUP,0," << info.super_block->s_blocks_count << ","
     << info.super_block->s_inodes_count << ","
     << info.des_table->bg_free_blocks_count << ","
     << info.des_table->bg_free_inodes_count << ","
     << info.des_table->bg_block_bitmap << ","
     << info.des_table->bg_inode_bitmap << ","
     << info.des_table->bg_inode_table << endl;

  Print(ss.str());
}


void getTime(time_t time, stringstream &ss){
  struct tm* timeinfo;
  timeinfo = gmtime(&time);
  ss << std::setfill('0') << std::setw(2)
     << timeinfo->tm_mon + 1<< "/"
     << std::setfill('0') << std::setw(2) 
     << timeinfo->tm_mday << "/"
     << std::setfill('0') << std::setw(2) 
     << (timeinfo->tm_year + 1900) % 100 << " "
     << std::setfill('0') << std::setw(2) 
     << timeinfo->tm_hour << ":"
     << std::setfill('0') << std::setw(2) 
     << timeinfo->tm_min << ":"
     << std::setfill('0') << std::setw(2) 
     << timeinfo->tm_sec << ",";
}

void process_inode(const ext2_inode &inode, __u32 inode_number){
  stringstream ss;
  ss << "INODE," << inode_number+1 << ",";
  if (S_ISREG(inode.i_mode))
    ss << "f,";
  else if (S_ISDIR(inode.i_mode))
    ss << "d,";
  else if (S_ISLNK(inode.i_mode))
    ss << "s,";
  else
    ss << "?,";
  //__u32 mode = inode.i_mode & 0xC;
  ss << std::oct << (inode.i_mode & 0x1FF) << ","
     << std::dec << inode.i_uid << ","
     << inode.i_gid << ","
     << inode.i_links_count << ",";
  getTime(inode.i_ctime, ss);
  getTime(inode.i_mtime, ss);
  getTime(inode.i_atime, ss);
  ss << inode.i_size << ","
     << inode.i_blocks << ",";
  for (int i = 0; i < EXT2_N_BLOCKS; i++){
    if (i == EXT2_N_BLOCKS - 1)
      ss << inode.i_block[i];
    else
      ss << inode.i_block[i] << ",";
  }
  ss << endl;
  Print(ss.str());
}

int process_indirect(const EXT2_info& info, ext2_inode& inode, __u32 index, int level, int block, int prev_block,  int& total) {
  // Read into __u32 buffer the current block
  int b_size = (EXT2_MIN_BLOCK_SIZE << info.super_block->s_log_block_size);
  int offset = SUPERBLOCK_OFFSET + (block - 1) * b_size;
  int limit = ceil( ((double)(b_size))/sizeof(__u32));
  __u32* buffer = new __u32[limit];
  Pread(info.image_fd, buffer, b_size, offset);


  stringstream ss;
 
  int first_offset = -1; // Remember first valid offset
    
  // For every entry so long as it is nonzero, print appropriate information
  
  for(int i = 0; i < limit /* && buffer[i]*/; i++) {
    if(!info.is_valid_block(buffer[i]))
      continue;
    if(level == 0){
      total += 1;
      offset = total;//SUPERBLOCK_OFFSET + (buffer[i] - 1) * b_size;
      
    }else
      offset = process_indirect(info, inode, index, level - 1, buffer[i], block, total);

    
    if(first_offset == -1) {
      first_offset = offset;
    }

    if(offset != -1) {
      ss << "INDIRECT," << index + 1 << ","
	 << level + 1 << ","
	 << offset << ","
	 << block  << ","
	 << buffer[i] << endl;
      Print(ss.str());
      ss.str(std::string()); // Clears sstream
    }
  }
  
  return first_offset;
}

void getIndirect(const EXT2_info& info, ext2_inode * inode_table, __u32 index) {
  ext2_inode& inode = inode_table[index];
  int total = -1;
  for(int i = 0; i < EXT2_N_BLOCKS/* && inode.i_block[i]*/; i++) {
    if(!info.is_valid_block(inode.i_block[i]))
      continue;
    switch(i){
    case EXT2_IND_BLOCK:
      process_indirect(info, inode, index, 0, inode.i_block[i], info.des_table->bg_inode_table, total);
      break;
    case EXT2_DIND_BLOCK:
      process_indirect(info, inode, index, 1, inode.i_block[i], info.des_table->bg_inode_table, total);
      break;
    case EXT2_TIND_BLOCK:
      process_indirect(info, inode, index, 2, inode.i_block[i], info.des_table->bg_inode_table, total);
      break;
    default:
      total += 1;
      //cerr << "Failure during indirect reference data collection." << endl;
      //exit(2);
      break;
    }
  }
  
}

__u32 DELETE_THIS;
void process_dir_recursive(const EXT2_info &info, const ext2_inode &inode, __u32 inode_number, __u32 level, __u32 block_number, __u32 &totalsize){
  DELETE_THIS = inode_number;
  __u32 block_size = EXT2_MIN_BLOCK_SIZE << info.super_block->s_log_block_size;
  __u32 offset = SUPERBLOCK_OFFSET + ((block_number - 1) * block_size);
  char *buffer = new char[block_size];
  Pread(info.image_fd, buffer, block_size, offset);
  stringstream ss;
  if (level == 3){
    __u32 index = 0;
    __u32 *blockNumbers = new __u32[block_size/sizeof(__u32)];
    blockNumbers = (__u32*) buffer;
    while(index < block_size/sizeof(__u32)){
      if (info.is_valid_block(blockNumbers[index]))
	process_dir_recursive(info, inode, inode_number, 2, blockNumbers[index], totalsize);
      index++;
    }
  }
  else if (level == 2){
    __u32 index = 0;
    __u32 *blockNumbers = new __u32[block_size/sizeof(__u32)];
    blockNumbers = (__u32*) buffer;
    while (index < block_size/sizeof(__u32)){
      if (info.is_valid_block(blockNumbers[index]))
	process_dir_recursive(info, inode, inode_number, 1, blockNumbers[index], totalsize);
      index++;
    }
  }
  else if (level == 1){
    __u32 index = 0;
    __u32 *blockNumbers = new __u32[block_size/sizeof(__u32)];
    blockNumbers = (__u32*) buffer;
    while (index < block_size/sizeof(__u32)){
      if (info.is_valid_block(blockNumbers[index]))
	process_dir_recursive(info, inode, inode_number,0, blockNumbers[index], totalsize);
      index++;
    }
  }
  else if (level == 0){
    ext2_dir_entry *file;
    file = (ext2_dir_entry*) buffer;
    __u32 size = 0;
    while (totalsize < inode.i_size && file->rec_len != 0 && size + file->rec_len <= block_size){
      if (file->inode && info.is_valid_inode(file->inode - 1)){
	ss << "DIRENT," << inode_number + 1 << "," << totalsize << ",";
	char file_name[EXT2_NAME_LEN + 1];
	ss << file->inode << ","
	   << file->rec_len << ","
	   << (int) file->name_len << ",";
	memcpy(file_name, file->name, file->name_len);
	file_name[file->name_len] = 0;
	ss << "'" << file_name << "'" << endl;
	size += file->rec_len;
	totalsize += file->rec_len;
	int amount = file->rec_len;
	char* temp = reinterpret_cast<char*>(file);
	temp += amount;
	file = reinterpret_cast<ext2_dir_entry*>(temp);
	Print(ss.str());
	ss.str(std::string());
      }else {
	size = size + file->rec_len;
	int amount = file->rec_len;
	char* temp = reinterpret_cast<char*>(file);
	temp += amount;
	file = reinterpret_cast<ext2_dir_entry*>(temp);
      }
    }
  }
  else{
    cerr << "ERROR" << endl;
  }
}

void process_dir(const EXT2_info &info, const ext2_inode &inode, __u32 inode_number){
  stringstream ss;
  __u32 block_number = 0;
  __u32 offset, size, totalsize = 0;
  ext2_dir_entry *file;
  __u32 block_size = EXT2_MIN_BLOCK_SIZE << info.super_block->s_log_block_size;
  char *buffer = new char[block_size];
  while (totalsize < inode.i_size && block_number < EXT2_NDIR_BLOCKS){
    if (!info.is_valid_block(inode.i_block[block_number])){
      block_number++;
      continue;
    }
    bzero(buffer, block_size);
    offset = SUPERBLOCK_OFFSET + ((inode.i_block[block_number] - 1) *block_size);
    Pread(info.image_fd,buffer,block_size,offset);
    size = 0;
    file = (ext2_dir_entry*) buffer;
    while (totalsize < inode.i_size && file->rec_len != 0 && (size + file->rec_len) <= block_size){
      if (file->inode && info.is_valid_inode(file->inode - 1)){
	ss << "DIRENT," << inode_number + 1 << "," << totalsize << ",";
	char file_name[EXT2_NAME_LEN + 1];
	ss << file->inode << ","
	   << file->rec_len << ","
	   << (int) file->name_len << ",";
	memcpy(file_name, file->name, file->name_len);
	file_name[file->name_len] = 0;
	ss << "'" << file_name << "'" << endl;
	size += file->rec_len;
	totalsize += file->rec_len;
	int amount = file->rec_len;
	char * temp = reinterpret_cast<char*>(file);
	temp += amount;
	file = reinterpret_cast<ext2_dir_entry*>(temp);
	Print(ss.str());
	ss.str(std::string());
      }else {
	size = size + file->rec_len;
	int amount = file->rec_len;
	char* temp = reinterpret_cast<char*>(file);
	temp += amount;
	file = reinterpret_cast<ext2_dir_entry*>(temp);
      }
    }
    block_number++;
  }

  while (totalsize < inode.i_size && block_number < EXT2_N_BLOCKS){
    if (block_number == EXT2_IND_BLOCK && info.is_valid_block(inode.i_block[block_number]))
      process_dir_recursive(info, inode, inode_number, 1, inode.i_block[block_number], totalsize);
    else if (block_number == EXT2_DIND_BLOCK && info.is_valid_block(inode.i_block[block_number]))
      process_dir_recursive(info, inode, inode_number, 2, inode.i_block[block_number], totalsize);
    else if (block_number == EXT2_TIND_BLOCK && info.is_valid_block(inode.i_block[block_number]))
      process_dir_recursive(info, inode, inode_number, 3, inode.i_block[block_number], totalsize);
    block_number++;
  }
    
}
					 
void getInode(const EXT2_info &info){
  off_t offset = SUPERBLOCK_OFFSET + ( (info.des_table->bg_inode_table - 1) * (EXT2_MIN_BLOCK_SIZE << info.super_block->s_log_block_size));
  __u32 inodes_per_block = ceil((double)(EXT2_MIN_BLOCK_SIZE << info.super_block->s_log_block_size) / sizeof(ext2_inode));
  __u32 inodes_blocks = info.super_block->s_inodes_per_group / inodes_per_block;
  __u32 size = inodes_blocks * (EXT2_MIN_BLOCK_SIZE << info.super_block->s_log_block_size);
  __u32 limit = ceil((double)(size)/sizeof(ext2_inode));
  ext2_inode* inode_table = new ext2_inode[limit];
  ssize_t rc = Pread(info.image_fd, inode_table, size, offset);
 
  for (__u32 i = 0; i < limit; i++) {
    if(!info.is_valid_inode(i))
      continue;
    if (inode_table[i].i_mode != 0 && inode_table[i].i_links_count != 0) {
      process_inode(inode_table[i], i);
      if (S_ISDIR(inode_table[i].i_mode))
	process_dir(info, inode_table[i], i);
      getIndirect(info, inode_table, i);
    }

  }
}

// @return Nonzero if bit is active, 0 otherwise.
inline char getBit(char * buffer, __u32 i){
  return (buffer[ i / (8 * sizeof(char)) ] & (0x1 << (i % (8 * sizeof(char))) ));
}

// Helper function for printing analysis for free block bitmap and free inode bitmap
void analyze_bitmap(const EXT2_info& info, char * buffer, off_t offset, string type){
  __u32 b_size = (EXT2_MIN_BLOCK_SIZE << info.super_block->s_log_block_size);
  ssize_t rc = Pread(info.image_fd, buffer, b_size, offset);

  stringstream ss;
  for(__u32 i = 0; i < b_size; i++){
    if(!getBit(buffer, i)){
      ss << type << (i + 1) << endl;
      Print(ss.str());
      ss.str(std::string()); // Clear stream
    }
  }
  
}

// Collects free block information using data bitmap
void getFreeBlock(const EXT2_info &info) {
  __u32 b_size = (EXT2_MIN_BLOCK_SIZE << info.super_block->s_log_block_size);
  off_t offset = SUPERBLOCK_OFFSET + (info.des_table->bg_block_bitmap - 1) * b_size;
  char * buffer = new char[b_size];
  analyze_bitmap(info, buffer, offset, "BFREE,");
}

// Collects free inode information using inode bitmap
void getFreeInode(const EXT2_info& info) {
  __u32 b_size = (EXT2_MIN_BLOCK_SIZE << info.super_block->s_log_block_size);
  off_t offset = SUPERBLOCK_OFFSET + (info.des_table->bg_inode_bitmap - 1) * b_size;
  char * buffer = new char[b_size];
  analyze_bitmap(info, buffer, offset, "IFREE,");
  
}

int main(int argc, char *argv[])
{
  string file = get_file_name(argc, argv);
  EXT2_info info;
  info.image_fd = Open(file.c_str(), O_RDONLY);

  getSuperblock(info);
  getGroupDescriptor(info);
  info.init_b_maps();
  getFreeBlock(info);
  getFreeInode(info);
  getInode(info);
  
  return 0;
}
