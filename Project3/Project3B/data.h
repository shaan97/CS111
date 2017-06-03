#ifndef DATA_H
#define DATA_H

#include <vector>
#include <unordered_map>

struct Inode {
    bool isAllocated = false;
    bool onFreeList = false;
  int linkcount = 0;
  int numLinks = 0;
};

struct Indirect {
    std::unordered_map<long, long> offsets; // Maps inode number to logical block offset
};

struct SuperBlock {
    long numBlocks;
    long numInodes;
    long nonreserved_block;
};

struct Block {
    std::vector<long> inodes;
    std::vector<long> offsets;
    bool onFreelist = false;
    std::vector<int> levels;
};



#endif
