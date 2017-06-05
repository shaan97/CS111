default: build
build:
	LD_LIBRARY_PATH=/usr/local/cs/gcc-5.2.0/lib64/:$LD_LIBRARY_PATH
	export LD_LIBRARY_PATH
	g++ lab3b.cpp -o lab3b
debug:
	LD_LIBRARY_PATH=/usr/local/cs/gcc-5.2.0/lib64/:$LD_LIBRARY_PATH
	export LD_LIBRARY_PATH
	g++ -g lab3b.cpp -o lab3b
clean:
	-rm -rf lab3b lab3b-904606576.tar.gz
dist:
	tar -czvf lab3b-904606576.tar.gz lab3b.cpp data.h Makefile README