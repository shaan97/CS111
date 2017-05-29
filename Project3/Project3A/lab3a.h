#ifndef LAB3A_H
#define LAB3A_H

#include <string>
#include "ext2_fs.h"

#define SUPERBLOCK_OFFSET 0x400
void getSuperblock(EXT2_info* info);
void getGroupDescriptor(EXT2_info* info);
void Print(std::string message);
void get

#endif
