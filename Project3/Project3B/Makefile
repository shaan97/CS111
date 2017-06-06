#NAME: SHAAN MATHUR, GARVIT PUGALIA
#EMAIL: shaankaranmathur@gmail.com, garvitpugalia@gmail.com
#ID: 904606576, 504628127

default: build
build:
	./build.sh 2> /dev/null
debug:
	g++ -g -std=c++11 -static-libstdc++ lab3b.cpp -o lab3b
clean:
	-rm -rf lab3b lab3b-904606576.tar.gz
dist:
	tar -czvf lab3b-904606576.tar.gz lab3b.cpp data.h Makefile README build.sh
