default: build
build: lab4c.o
	gcc -lmraa -lm lab4c.o -o lab4c
lab4c.o: lab4c.c
	gcc -c lab4c.c -o lab4c.o
debug: lab4c.o
	gcc -g lab4c.o -o lab4c
clean:
	-rm -rf *.o lab4c
