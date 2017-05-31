#NAME: Shaan Mathur, Garvit Pugalia
#EMAIL: shaankaranmathur@gmail.com, garvitpugalia@gmail.com
#ID: 904606576, 504628127

default: build
build: lab3a.cpp EXT2_info.cpp
	-LD_LIBRARY_PATH=/usr/local/cs/gcc-5.2.0/lib64/:$LD_LIBRARY_PATH
	-export LD_LIBRARY_PATH
	g++ EXT2_info.cpp lab3a.cpp -o lab3a -lm
debug:
	-LD_LIBRARY_PATH=/usr/local/cs/gcc-5.2.0/lib64/:$LD_LIBRARY_PATH
	-export LD_LIBRARY_PATH
	g++ -g EXT2_info.cpp lab3a.cpp -o lab3a -lm
clean:
	rm -rf lab3a
dist:
	tar -czvf lab3a-904606576.tar.gz EXT2_info.cpp lab3a.cpp EXT2_info.h Ext2_fs.h Makefile ext2_fs.h lab3a.h README
