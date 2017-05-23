default: lab4b.c
	gcc -lm -lmraa lab4b.c -o lab4b
clean:
	-rm -rf lab4b
debug:
	gcc -lm -lmraa -g lab4b.c -o lab4b
check:
	./test.sh
dist:
	tar -czvf lab4b-904606576.tar.gz lab4b.c README test.sh Makefile
