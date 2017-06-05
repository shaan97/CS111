#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <sstream>
#include <algorithm>
#include <climits>
#include <cmath>

#include "data.h"


using namespace std;


// Prints Usage Statement detailing how to send valid command line arguments
// to the executable
inline void print_usage() {
	cerr << "Usage: ./lab3b [FILENAME]" << endl;
}

// Initializes Input Stream.
// Expected input format: [FILENAME]
void init_file(int argc, char ** argv, ifstream& fin) {
	if(argc != 2) {
		cerr << "Illegal Number of Arguments." << endl;
		print_usage();
		exit(1);
	}

	fin.open(argv[1]);
	if(fin.rdstate() == ifstream::failbit) {
		// Error opening file
		cerr << "Error opening file." << endl;
		print_usage();
		exit(1);
	}

}

void ignore_num(istream& in, int numWords) {
	string trash;
	while(numWords-- && in) {
		in >> trash;
	}
}


/*  Collects all requisite data from input file into data structures
	necessary for analysis.

	@param fin			    Input file stream. Expected to be in format 
							specified in Project3A

	@param blocks		    Will be modified to map inode number to Block information.
							See "data.h" for details on the struct

	@param super			Will be modified to contain Superblock data.
							See "data.h" for details on the struct

	@param indir			Will be modified to contain mapping from block number to logical block offset.
							See "data.h" for details on the struct. Will NEVER have an invalid offset. Insert
							only if offset is known.

	@param inodes		    Will be modified to contain mapping from inode number to information on Inode.
							See "data.h" for details on the struct

    @param gp               Will be modified to contain information in Group block.
                            See "data.h" for details on the struct

    @param dirs             Will contain mapping from inode number to info on directory.
                            See "data.h" for details on the struct
 */

void collect_data(ifstream& fin, unordered_map<long, Block>& blocks, SuperBlock& super, 
				unordered_multimap<long, Indirect>& indir, unordered_multimap<long, Inode>& inodes, 
                Group& gp, unordered_map<long, Directory>& dirs) {
	string next, line, line2;
	stringstream ss;
	while(getline(fin, line)) {
        line2 = line;
        // TODO : Handle cases where ',' is in file name
        replace(line.begin(), line.end(), ',', ' '); // Setup whitespace for the stringstream
		ss << line; // Send first line into stringstream so we can parse more easily
		
		// Determine nature of line
		/* TODO : Make sure stream is always valid during insertions. */
		ss >> next;
		if(next == "SUPERBLOCK") {
			ss >> super.numBlocks;
			ss >> super.numInodes;
			ss >> super.blockSize;
			ss >> super.inodeSize;
			ignore_num(ss, 1); // Ignore next word
			ss >> super.inodesPerGroup;
			ss >> super.nonreserved_inode;

		} else if(next == "GROUP") {
			long group_num, freeBlocks, firstBlock;
			ss >> group_num;

			ignore_num(ss, 1);
            ss >> gp.numInodes;
            ss >> freeBlocks;
            ignore_num(ss, 3);
            ss >> firstBlock;
			/* TODO : Handle multiple groups */
			gp.freeBlocks = freeBlocks;
			gp.num = group_num;
			gp.firstBlock = firstBlock;
			/*
			Group gp;
			gp.freeBlocks = freeBlocks;
			groups.emplace(group_num, gp);
			*/
		} else if(next == "BFREE") {
			Block b;
			b.onFreelist = true;
			long block_num;
			ss >> block_num;


			auto itr = blocks.find(block_num);
			if(itr != blocks.end()) {
				// Already in map
				itr->second.onFreelist = true;
			} else {
				// Not in map, add it
				blocks.insert(make_pair(block_num, b));
			}

		} else if(next == "IFREE") {
			Inode i;
			long inode_num;
			ss >> inode_num;
			i.num = inode_num;
			auto range = inodes.equal_range(inode_num);
			if (range.first == range.second)
			{
				i.onFreeList = true;
				inodes.emplace(inode_num, i);
			}
			else {
				auto itr = range.first;
				while (itr != range.second) {
					itr->second.onFreeList = true;
					itr->second.num = inode_num;
					++itr;
				}
			}
		}
		else if (next == "INODE")
		{
			Inode i;
			long inode_num;
			int linkcount;
			ss >> inode_num;
			i.num = inode_num;
            ss >> i.type;
            ignore_num(ss, 3);
            ss >> linkcount;
			auto range = inodes.equal_range(inode_num);
			if (range.first == range.second)
			{
				i.isAllocated = true;
				i.linkcount = linkcount;
				inodes.emplace(inode_num, i);
			}
			else
			{
				auto itr = range.first;
				while(itr != range.second) {
					itr->second.isAllocated = true;
					itr->second.linkcount = linkcount;
					itr->second.num = inode_num;
                	itr->second.type = i.type;
					++itr;
				}
				
			}

            Block b;

			ignore_num(ss, 8); // Ignore the next 8 tokens to jump to block numbers

			long block_num, count = 0;
			while(ss >> block_num) { // Indirect cases should be handled at resolve_offset once SUPERBLOCK info has been processed
				
				if(!block_num) {
					// Ignore 0, but still increment offset
					count++;
					continue;
				}
				
				auto itr = blocks.find(block_num);
				if(itr == blocks.end()) {
					// Not in in map
					auto p = blocks.emplace(block_num, b);
					itr = p.first;
				}
				
				itr->second.inodes.push_back(inode_num);
				int offset = count < 12 ? count : -2;
				itr->second.offsets.push_back(offset);
				int level = count < 12 ? 0 : count % 11;
				itr->second.levels.push_back(level);
				
				
				count++;
			}

		} else if(next == "DIRENT") {
			Inode i;
			long inode_num;

			Directory dir;
			ss >> dir.num;
			ignore_num(ss, 1);
			ss >> inode_num;
			i.num = inode_num;

			auto range = inodes.equal_range(inode_num);
			ignore_num(ss, 2);
            
            i.name = line2.substr(line2.find("'"));
            while(i.name[i.name.size() - 1] != '\'')
                i.name.pop_back();

			if (i.name == "'.'")
				i.currentDirNum = i.num;

			std::unordered_multimap<long, Inode>::iterator itr;
			if (range.first == range.second)
			{
				i.numLinks = 1;
				itr = inodes.emplace(inode_num, i);
			}
			else
			{
				itr = range.first;
				i.numLinks = itr->second.numLinks + 1;
				i.onFreeList = itr->second.onFreeList;
				i.type = itr->second.type;
				i.linkcount = itr->second.linkcount;
				i.isAllocated = itr->second.isAllocated;
				
				while (itr != range.second)
				{
					itr->second.numLinks++;
					itr->second.num = i.num;
					if(i.name == "'.'")
						itr->second.currentDirNum = i.num;
					else if(itr->second.name == "'.'")
						i.currentDirNum = itr->second.currentDirNum;
					++itr;
				}

				itr = inodes.emplace(inode_num, i);
			}

			auto d = dirs.find(dir.num);
			

			if (d != dirs.end())
			{
				// There exists a directory with this directory number
				auto begin = d->second.entries.begin();
				if (i.name == "'.'")
				{
					for_each(begin, d->second.entries.end(), [&i](pair<const long, Inode> &p) {
						p.second.currentDirNum = i.num;
					});
				} else if(begin->second.currentDirNum > 0) {
					i.currentDirNum = begin->second.currentDirNum;
				}
				d->second.entries.emplace(i.num, i);
				/*auto entry_range = d->second.entries.equal_range(i.num); //emplace(itr->second.num, itr->second);
				if(entry_range.first == entry_range.second) {
					d->second.entries.emplace(i.num, i);
				}
				else {
					if(i.name == "'.'") {
						for_each(entry_range.first, entry_range.second, [&i](pair<const long, Inode> &p) {
							p.second.currentDirNum = i.num;
						});
					} else if(entry_range.first->second.currentDirNum > 0) {
						// currentDirNum has been assigned
						i.currentDirNum = entry_range.first->second.currentDirNum;
					}
					d->second.entries.emplace(i.num, i);
				}*/
			} else {
				// Directory does not yet exist
                dir.entries.emplace(i.num, i);
                dirs.emplace(dir.num, dir);
            }            
        }
		else if (next == "INDIRECT")
		{
			long inode, level, offset, block;
			ss >> inode >> level >> offset >> block;
			ss >> block;
			Indirect ind;
			ind.level = level;
			ind.offset = offset;
			ind.inode = inode;
			
			indir.emplace(block, ind);
			
			/*
			====================================== DEPRECATED CODE ========================================
			The following code attempts to take the second to last entry (which is the block number of the
			block that has a reference to the offset). This is an unnecessary step. So long as we take
			the final entry as the block number and its (guaranteed-to-be-correct) offset, we will get all
			indirect block->offsets for indirect blocks not explicitly in the inode. For that extra case,
			our analysis will resolve the offset using some mathematics.
			===============================================================================================
			auto itr = blocks.find(block);
			if(itr == blocks.end()) {
				Block b;
				auto res = blocks.emplace(block, b);
				itr = res.first;
				itr->second.inodes.push_back(inode);
				itr->second.levels.push_back(level);
				itr->second.offsets.push_back(ind.offset);
			} else {
				// If indirect entry exists, let's make sure it is of minimum offset. Otherwise insert it
				bool found = false;
				for(int k = 0; k < itr->second.inodes.size(); k++) {
					if(itr->second.inodes[k] == inode && itr->second.levels[k] == level) {
						itr->second.offsets[k] = std::min(itr->second.offsets[k], ind.offset);
						found = true;
						break;
					}
				}

				if(!found) {
					Block b;
					auto res = blocks.emplace(block, b);
					itr = res.first;
					itr->second.inodes.push_back(inode);
					itr->second.levels.push_back(level);
					itr->second.offsets.push_back(ind.offset);
				}
			}
			*/

			// The data block that an indirect pointer points to will never be referred to elsewhere, so we must capture it now
			
			auto itr2 = blocks.find(block);
			if (itr2 == blocks.end()) {
				Block b;
				auto res = blocks.emplace(block, b);
				itr2 = res.first;
			}
			

			// No risk of duplication because this inode/level/offset triplet can only appear once (e.g. INDIRECT,10,1,...,34,35
			// cannot have a second reference)
			itr2->second.inodes.push_back(inode);
			itr2->second.levels.push_back(level);
			itr2->second.offsets.push_back(ind.offset);
		} else {
			// Bad Input
			cerr << "File is either not a properly formatted .csv or this is the wrong file." << endl;
			print_usage();
			exit(1);
		}

		ss.clear();
		ss.str(std::string());
	}
}

long resolve_offset(const unordered_multimap<long, Indirect> &indir, unordered_map<long,Block>::const_iterator itr, long j, const SuperBlock& super)
{
	auto range = indir.equal_range(itr->first);
	auto begin = range.first;
	auto end = range.second;

	while (begin != end)
	{
		if (begin->second.level == itr->second.levels[j] && begin->second.inode == itr->second.inodes[j])
			break;
		++begin;
	}

	if(begin != end)
		return begin->second.offset;
	else { 
		// Will not work for unresolved multi-indirect (offset == -1 && level > 1) that is invalid
		long sum = 0;
		switch(itr->second.levels[j]) {
		case 0:
			return 12;
		case 3:
			sum += pow(super.blockSize / sizeof(__uint32_t), 2);
		case 2:
			sum += pow(super.blockSize / sizeof(__uint32_t), 1);
		case 1:
			sum += pow(super.blockSize / sizeof(__uint32_t), 0) + 11;
		
		}
		return sum;
	}
}

void audit_inodes(unordered_multimap<long, Inode> &inodes, unordered_map<long, Directory>& dirs, const SuperBlock& super, const Group& gp) {
    for (long i = super.nonreserved_inode; i <= gp.numInodes; i++)
    {
        auto range = inodes.equal_range(i);
        if (range.first == range.second)
            cout << "UNALLOCATED INODE " << i << " NOT ON FREELIST" << endl;
    }

    for (auto itr = inodes.begin(); itr != inodes.end();) {
        if (itr->second.isAllocated == true && itr->second.onFreeList == true)
			cout << "ALLOCATED INODE " << itr->first << " ON FREELIST" << endl;
		if (itr->second.isAllocated == true && itr->second.linkcount != itr->second.numLinks)
			cout << "INODE " << itr->first << " HAS " << itr->second.numLinks << " LINKS BUT LINKCOUNT IS " << itr->second.linkcount << endl;

		std::advance(itr, inodes.count(itr->first)); // Skip over duplicate keys
	}

	const long ROOT = 2;
	unordered_map<long, long> parentDir;
	parentDir.emplace(ROOT, ROOT);
	for (auto dir = dirs.begin(); dir != dirs.end(); ++dir)
	{
		auto entry = dir->second.entries.begin();
		if (entry->second.currentDirNum != dir->second.num && entry->second.currentDirNum != -1)
			cout << "DIRECTORY INODE " << dir->second.num << " NAME '.' LINK TO INODE " << entry->second.currentDirNum << " SHOULD BE " << dir->second.num << endl;

		for (; entry != dir->second.entries.end(); ++entry)
		{
			auto range = inodes.equal_range(entry->second.num);
			if (range.first == range.second)
			{
				cout << "DIRECTORY INODE " << dir->second.num << " NAME " << entry->second.name << " INVALID INODE " << entry->second.num << endl;
			}
			else if (!range.first->second.isAllocated)
			{
				cout << "DIRECTORY INODE " << dir->second.num << " NAME " << entry->second.name << " UNALLOCATED INODE " << entry->second.num << endl;
			}

			if(entry->second.type == 'd' && entry->second.name != "'.'" && entry->second.name != "'..'") {
				parentDir.emplace(entry->second.num, dir->second.num);
			}
		}
	}

	for (auto dir = dirs.begin(); dir != dirs.end(); ++dir) {
		auto entry = find_if(dir->second.entries.begin(), dir->second.entries.end(), [](pair<const long, Inode> &p) {
			return p.second.name == "'..'";
		});
		auto parent = parentDir.find(dir->second.num);

		if (entry != dir->second.entries.end() && parent != parentDir.end() && parent->second != entry->second.num)
		{
			cout << "DIRECTORY INODE " << parent->second << " NAME '..' LINK TO INODE " << entry->second.num << " SHOULD BE " << parent->second << endl;
		}
	}
}

void audit_block(unordered_map<long, Block>& blocks, const SuperBlock& super, const Group& gp, unordered_multimap<long, Indirect>& indir) {

	// Keep track of which valid blocks have not been referenced yet
	unordered_set<long> unref_blocks;
	const long FIRST_U_BLOCK = gp.firstBlock + (super.inodesPerGroup * super.inodeSize) / super.blockSize;
	for(long i = FIRST_U_BLOCK; i < super.numBlocks; i++) {
		unref_blocks.insert(i);
	}


	for(auto itr = blocks.begin(); itr != blocks.end(); ++itr) {
		unref_blocks.erase(itr->first); // Block has been referenced
		if(itr->second.onFreelist && itr->second.inodes.size())
			cout << "ALLOCATED BLOCK " << itr->first << " ON FREELIST" << endl; 
		for(long j = 0; j < itr->second.inodes.size(); j++) {
			string type; // Defaults to empty string
			switch(itr->second.levels[j]) {
			case 1:
				type = "INDIRECT ";
				break;
			case 2:
				type = "DOUBLE INDIRECT ";
				break;
			case 3:
				type = "TRIPPLE INDIRECT ";
				break;
			}

			if(itr->first < FIRST_U_BLOCK) {
				// RESERVED BLOCK
				cout << "RESERVED " << type << "BLOCK " << itr->first << " IN INODE " << itr->second.inodes[j] << " AT OFFSET ";
				long off = 12;
				if(itr->second.offsets[j] < 0) {
					// Offset is unknown and indirect
					
					off = resolve_offset(indir, itr, j, super);
					itr->second.offsets[j] = off; // Resolves offset for later evaluation

				} else {
					off = itr->second.offsets[j];
				}
				cout << off << endl;
				
			} else if(itr->first >= super.numBlocks || itr->first < 0) {
				
				cout << "INVALID " << type << "BLOCK " << itr->first << " IN INODE " << itr->second.inodes[j] << " AT OFFSET ";
				long off = 12;
				if(itr->second.offsets[j] < 0) {
					// Offset is unknown and indirect

					off = resolve_offset(indir, itr, j, super);
					
					itr->second.offsets[j] = off; // Resolves offset for later evaluation
				}

				cout << itr->second.offsets[j] << endl;
			}
		}

		if(itr->second.inodes.size() > 1) {
			// Multiple instances of the block were detected
			for(long i = 0; i < itr->second.inodes.size(); i++) {
				string type;
				switch(itr->second.levels[i]) {
				case 1:
					type = "INDIRECT ";
					break;
				case 2:
					type = "DOUBLE INDIRECT ";
					break;
				case 3:
					type = "TRIPPLE INDIRECT ";
					break;
				}
				cout << "DUPLICATE " << type << "BLOCK " << itr->first << " IN INODE " << itr->second.inodes[i] << " AT OFFSET ";
				if(itr->second.offsets[i] < 0) {
					itr->second.offsets[i] = resolve_offset(indir, itr, i, super);
				}

				cout << itr->second.offsets[i] << endl;

			}
		}
	}

	for(auto itr = unref_blocks.begin(); itr != unref_blocks.end(); ++itr) {
		cout << "UNREFERENCED BLOCK " << *itr << endl; 
	}
}

int main(int argc, char ** argv) {
	ifstream fin;
	unordered_map<long, Block> blocks;
	unordered_multimap<long, Indirect> indir;
	unordered_multimap<long, Inode> inodes;
	SuperBlock super;
	Group gp;
	unordered_map<long, Directory> dirs;
	// Initialize File and Input Stream
	init_file(argc, argv, fin);

	collect_data(fin, blocks, super, indir, inodes, gp, dirs);
	audit_block(blocks, super, gp, indir);
	audit_inodes(inodes, dirs, super, gp);

	return 0;
}
