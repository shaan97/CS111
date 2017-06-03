#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <sstream>
#include <algorithm>
#include <climits>

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

    @param fin              Input file stream. Expected to be in format 
                            specified in Project3A
    @param blocks           Will be modified to map inode number to Block information.
                            See "data.h" for details on the struct
    @param super            Will be modified to contain Superblock data.
                            See "data.h" for details on the struct
    @param indir            Will be modified to contain mapping from inode number to logical block offset.
                            See "data.h" for details on the struct
    @param inodes           Will be modified to contain mapping from inode number to information on Inode.
                            See "data.h" for details on the struct
 */

void collect_data(ifstream& fin, unordered_map<long, Block>& blocks, SuperBlock& super, Indirect& indir, unordered_map<long, Inode>& inodes) {
    string next, line;
    stringstream ss;
    while(fin){
        
        getline(fin, line);
        replace(line.begin(), line.end(), ',', ' '); // Setup whitespace for the stringstream
        ss << line; // Send first line into stringstream so we can parse more easily
        
        // Determine nature of line
        /* TODO : Make sure stream is always valid during insertions. */
        ss >> next;
        if(next == "SUPERBLOCK") {
            ss >> super.numBlocks;
            ss >> super.numInodes;
            ignore_num(ss, 4); // Ignore next four words
            ss >> super.nonreserved_block;

        } else if(next == "GROUP") {

        } else if(next == "BFREE") {
            Block b;
            b.onFreelist = true;
            long block_num;
            ss >> block_num;
            b.levels.push_back(-1); // Flag that level is invalid

            auto itr = blocks.find(block_num);
            if(itr != blocks.end()) {
                // Already in map
                itr->second.onFreelist = true;
            } else {
                // Not in map, add it
                blocks.insert(make_pair(block_num, b));
            }

        } else if(next == "IFREE") {

        } else if(next == "INODE") {
            Block b;
            long inode_num;
            ss >> inode_num;
            ignore_num(ss, 13); // Ignore the next 13 tokens to jump to block numbers

            long block_num, count = 0;
            while(ss) {
                ss >> block_num;
                
                auto itr = blocks.find(block_num);
                if(itr == blocks.end()) {
                    // Not in in map
                    auto p = blocks.emplace(block_num, b);
                    itr = p.first;
                }
                
                itr->second.inodes.push_back(inode_num);
                if(count < 12) {
                    itr->second.offsets.push_back(count);
                    itr->second.levels.push_back(0);
                } else {
                    itr->second.levels.push_back(count - 11);
                    itr->second.offsets.push_back(-1); // Currently unknown. Flag to check indirect data later        
                }
                
                count++;
            }

        } else if(next == "DIRENT") {

        } else if(next == "INDIRECT") {

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

void resolve_offset(Block& b, const Indirect& indir);

void audit_block(const unordered_map<int, Block>& blocks, const SuperBlock& super, Indirect& indir) {
    // Keep track of which valid blocks have not been referenced yet
    unordered_set<int> unref_blocks;
    for(long i = super.nonreserved_block; i < super.numBlocks; i++) {
        unref_blocks.insert(i);
    }

    for(auto itr = blocks.begin(); itr != blocks.end(); ++itr) {
        unref_blocks.erase(itr->first); // Block has been referenced
        for(long j = 0; j < itr->second.inodes.size(); j++) {
            string type; // Defaults to empty string
            switch(itr->second.levels[j]) {
            case 1:
                type = "INDIRECT";
                break;
            case 2:
                type = "DOUBLE INDIRECT";
                break;
            case 3:
                type = "TRIPLE INDIRECT";
                break;
            }

            if(itr->first < super.nonreserved_block) {
                // RESERVED BLOCK
                cout << "RESERVED " << type << " BLOCK " << itr->first << " IN INODE " << itr->second.inodes[j] << " AT OFFSET "; 
                if(itr->second.offsets[j] == -1)
                    resolve_offset(itr->second, indir);
            } else if(itr->first >= super.numBlocks){
                // INVALID BLOCK
            }
        }

        if(itr->second.inodes.size() > 1) {
            // Block is held by multiple inodes
            for(long i = 0; i < itr->second.inodes.size(); i++) {
                cout << "DUPLICATE BLOCK "
            }
        }
    }


}

int main(int argc, char ** argv) {
    ifstream fin;
    unordered_map<int, Block> blocks;
    SuperBlock super;

    // Initialize File and Input Stream
    init_file(argc, argv, fin);

    collect_data(fin, blocks, super);

    return 0;
}