default: hash.o SortedList.o lab2_list.o
	gcc -g -pthread hash.o SortedList.o lab2_list.o -o lab2_list
hash.o: hash.c
	gcc -g -c hash.c
SortedList.o: SortedList.c
	gcc -g -c SortedList.c
lab2_list.o: lab2_list.c
	gcc -g -c lab2_list.c
tests:
	./test.sh
	-./lab2_list --lists=4 --threads=4 --iterations=1 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=4 --iterations=2 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=4 --iterations=4 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=4 --iterations=8 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=4 --iterations=16 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=8 --iterations=1 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=8 --iterations=2 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=8 --iterations=4 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=8 --iterations=8 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=8 --iterations=16 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=12 --iterations=1 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=12 --iterations=2 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=12 --iterations=4 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=12 --iterations=8 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=12 --iterations=16 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=16 --iterations=1 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=16 --iterations=2 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=16 --iterations=4 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=16 --iterations=8 --yield=id >> lab2b_list.csv
	-./lab2_list --lists=4 --threads=16 --iterations=16 --yield=id >> lab2b_list.csv
graphs:
	gnuplot ./lab2_list.gp

clean:
	rm -f lab2_list *.csv *.png *.o

profile:
	-rm -f ./raw.gperf
	LD_PRELOAD=/u/cs/ugrad/shaan/lib/libprofiler.so.0 CPUPROFILE=raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=spin_insert ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf

dist:
	tar -czvf lab2b-904606576.tar.gz lab2_list.c SortedList.h SortedList.c hash.h hash.c lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png lab2b_list.csv lab2_list.gp Makefile profile.out README test.sh
