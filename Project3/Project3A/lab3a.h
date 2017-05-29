#ifndef LAB3A_H
#define LAB3A_H


#include "Ext2_fs.h" // Wrapper for ext2_fs.h
#include <string>
#include "EXT2_info.h"


#define SUPERBLOCK_OFFSET 0x400
void getSuperblock(EXT2_info* info);
void getGroupDescriptor(EXT2_info* info);
void Print(std::string message);


#endif
