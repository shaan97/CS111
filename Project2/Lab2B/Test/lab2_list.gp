#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2_list-1.png ... cost per operation vs threads and iterations
#	lab2_list-2.png ... threads and iterations that run (un-protected) w/o failure
#	lab2_list-3.png ... threads and iterations that run (protected) w/o failure
#	lab2_list-4.png ... cost per operation vs number of threads
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#

# general plot parameters
set terminal png
set datafile separator ","


set title "List-1: Total Operations Per Second"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput (Operations / Second)"
set logscale y 10
set output 'lab2b_1.png'

# grep out only single threaded, un-protected, non-yield results
plot \
     "< grep 'list-none-s,[0-9]*,[0-9]*,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin-Lock' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,[0-9]*,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex' with linespoints lc rgb 'blue'


set title "List-2: Wait-For-Lock and Actual Operation Time"
set xlabel "Threads"
set logscale x 2
set ylabel "Time"
set logscale y 10
set output 'lab2b_2.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep 'list-none-m,[0-9]*,[0-9]*,1,' lab2b_list.csv" using ($2):($7) \
	title 'Average Time Per Operation' with linespoints lc rgb 'green', \
     "< grep 'list-none-s,[0-9]*,[0-9]*,1,' lab2b_list.csv" using ($2):($8) \
	title 'Wait-For-Lock Time' with linespoints lc rgb 'red'


set title "List-3: 4 List Implementation - Successes"
set xlabel "Threads"
set logscale x 2
set ylabel "Iterations"
set logscale y 10
set output 'lab2b_3.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'No Synchronization' with points lc rgb 'green', \
     "< grep 'list-id-[sm]*,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Synchronization' with points lc rgb 'red'



set title "List-4: Throughput With Partitioned Lists (Mutex)"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput (Operations / Second)"
set logscale y 10
set output 'lab2b_4.png'

# grep out only single threaded, un-protected, non-yield results
plot \
     "< grep 'list-none-m,[0-9]*,[0-9]*,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 List' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 Lists' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,[0-9]*,[0-9]*,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 Lists' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,[0-9]*,[0-9]*,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 Lists' with linespoints lc rgb 'yellow'

set title "List-5: Throughput With Partitioned Lists (Spin-Locks)"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput (Operations / Second)"
set logscale y 10
set output 'lab2b_5.png'

# grep out only single threaded, un-protected, non-yield results
plot \
     "< grep 'list-none-s,[0-9]*,[0-9]*,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 List' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 Lists' with linespoints lc rgb 'green', \
     "< grep 'list-none-s,[0-9]*,[0-9]*,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 Lists' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,[0-9]*,[0-9]*,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 Lists' with linespoints lc rgb 'yellow'
