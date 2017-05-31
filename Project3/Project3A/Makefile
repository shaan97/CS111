#NAME: Shaan Mathur, Garvit Pugalia
#EMAIL: shaankaranmathur@gmail.com, garvitpugalia@gmail.com
#ID: 904606576, 504628127

default: build
build: lab3a.cpp
	g++ EXT2_info.cpp lab3a.cpp -o lab3a -lm
debug:
	g++ -g EXT2_info.cpp lab3a.cpp -o lab3a -lm
clean:
	rm -rf lab3a
dist:
	tar -czvf lab3a-904606576.tar.gz EXT2_info.cpp lab3a.cpp EXT2_info.h Ext2_fs.h Makefile ext2_fs.h lab3a.h README
