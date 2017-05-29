#IFNDEF LAB3A_H
#DEFINE LAB3A_H

#define SUPERBLOCK_OFFSET 0x400
enum FILE_DATA = { SUPERBLOCK, GROUP, BFREE, IFREE, INODE, DIRENT, INDIRECT};
void getSuperblock(EXT2_info* info);
void getGroupDescriptor(EXT2_info* info);
void Print(FILE_DATA type, std::string message);
