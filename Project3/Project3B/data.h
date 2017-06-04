#ifndef DATA_H
#define DATA_H

#include <vector>
#include <unordered_map>

struct Inode {
    bool isAllocated = false;
    bool onFreeList = false;
};

struct Indirect {
    long offset;
    long level;
    long inode;
};

struct Group {
    long num;
    long freeBlocks;
    long firstBlock;
};

struct SuperBlock {
    long numBlocks;
    long numInodes;
    long nonreserved_inode;
    long inodeSize = -1;
    long blockSize;
    long inodesPerGroup;
};

struct Block {
    std::vector<long> inodes;
    std::vector<long> offsets;
    bool onFreelist = false;
    std::vector<int> levels;
};



#endif