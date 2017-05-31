//NAME: Shaan Mathur, Garvit Pugalia
//EMAIL: shaankaranmathur@gmail.com, garvitpugalia@gmail.com
//ID: 904606576, 504628127

#ifndef LAB3A_H
#define LAB3A_H


#include "Ext2_fs.h" // Wrapper for ext2_fs.h
#include <string>

#include <unistd.h>
#include "EXT2_info.h"

#define SUPERBLOCK_OFFSET 0x400
void getSuperblock(EXT2_info &info);
void getGroupDescriptor(EXT2_info &info);
void getFreeBlock(const EXT2_info &info);
void getFreeInode(const EXT2_info &info);
void Print(const std::string message);
void getInode(const EXT2_info &info);
void getIndirect(const EXT2_info& info, ext2_inode * inode_table, __u32 index);
ssize_t Pread(int fd, void *buf, size_t count, off_t offset);
void process_dir(const EXT2_info &info, const ext2_inode &inode, __u32 inode_number);
void process_inode(const ext2_inode &inode, __u32 inode_number);
int process_indirect(const EXT2_info& info, ext2_inode& inode, __u32 index, int level, int block, int prev_block,  int& total);

#endif
