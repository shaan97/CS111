#ifndef DATA_H
#define DATA_H

#include <vector>

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