#!/bin/bash

make
./lab3a $1
sort lab3a.csv > lab3a_sort.csv
sort $2 > other_sort.csv
diff -u lab3a_sort.csv other_sort.csv
rm -rf lab3a_sort.csv other_sort.csv

