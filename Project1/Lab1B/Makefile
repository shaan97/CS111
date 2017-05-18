# NAME: SHAAN MATHUR
# UID: 904606576
# EMAIL: SHAANKARANMATHUR@GMAIL.COM

default:
	gcc -lmcrypt crypt.c lab1b-client.c -o lab1b-client
	gcc -lmcrypt crypt.c lab1b-server.c -o lab1b-server
lab1b-client:
	gcc -lmcrypt crypt.c lab1b-client.c -o lab1b-client
lab1b-server:
	gcc -lmcrypt crypt.c lab1b-server.c -o lab1b-server
clean:
	rm lab1b-client lab1b-server lab1b-904606576.tar.gz
dist:
	tar -czf lab1b-904606576.tar.gz lab1b-client.c lab1b-server.c crypt.c crypt.h Makefile README my.key
