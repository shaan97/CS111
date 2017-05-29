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
ssize_t Pread(int fd, void *buf, size_t count, off_t offset);
void process_dir();


#endif
