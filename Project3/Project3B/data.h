//NAME: SHAAN MATHUR, GARVIT PUGALIA
//EMAIL: shaankaranmathur@gmail.com, garvitpugalia@gmail.com
//ID: 904606576, 504628127

#ifndef DATA_H
#define DATA_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>



struct Inode {
    long num;
    int linkcount = 0;
    int numLinks = 0;
	bool isAllocated = false;
	bool onFreeList = false;
    std::string name;
	long currentDirNum = -1;
	char type = '?';
};

struct Directory {
    long num;
    std::unordered_multimap<long, Inode> entries;
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
    long numInodes;
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
