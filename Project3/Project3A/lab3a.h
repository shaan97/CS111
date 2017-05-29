#ifndef LAB3A_H
#define LAB3A_H

#include <string>
#include <unistd.h>
#include "ext2_fs.h"

#define SUPERBLOCK_OFFSET 0x400
void getSuperblock(EXT2_info &info);
void getGroupDescriptor(EXT2_info &info);
void Print(std::string message);
void readImap(const EXT2_info &info);
void readDmap(const EXT2_info &info);
void readInode(const EXT2_info &info);
ssize_t Pread(int fd, void *buf, size_t count, off_t offset);
void process_dir();


#endif
