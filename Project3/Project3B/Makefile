default: build
build:
	g++ -std=c++11 -static-libstdc++ lab3b.cpp -o lab3b
debug:
	g++ -g -std=c++11 -static-libstdc++ lab3b.cpp -o lab3b
clean:
	-rm -rf lab3b lab3b-904606576.tar.gz
dist:
	tar -czvf lab3b-904606576.tar.gz lab3b.cpp data.h Makefile README
