# NAME: SHAAN MATHUR
# EMAIL: SHAANKARANMATHUR@GMAIL.COM
# UID: 904606576


default: build
build: lab4c.o
	gcc -lmraa -lm -lssl -lcrypto lab4c.o -o lab4c_tcp
	-rm -rf lab4c_tls
	ln -s lab4c_tcp lab4c_tls
lab4c.o: lab4c.c
	gcc -c lab4c.c -o lab4c.o
debug:
	gcc -g -lmraa -lm -lssl -lcrypto lab4c.c -o lab4c_tcp
	-rm -rf lab4c_tls
	ln -s lab4c_tcp lab4c_tls
clean:
	-rm -rf *.o lab4c_* *.tar.gz
dist:
	tar -czf lab4c-904606576.tar.gz Makefile README lab4c.c
