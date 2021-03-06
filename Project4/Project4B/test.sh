#!/bin/bash

make

let errors=0

# Try dummy arguments
./lab4b --bogus > /dev/null
if [ $? -eq 0 ]
then
   echo "FAILURE. Allowed bogus arguments."
   let errors=errors+1
fi

# Try basic program
./lab4b --log=log.txt <<-EOF
PERIOD=2
SCALE=C
STOP
START
OFF
EOF

if [ $? -ne 0 ]
then
    echo "FAILURE. Demo program did not succeed."
    let errors=errors+1
fi

# Look for output in STDERR
touch error.txt
./lab4b --period=2 --log=log.txt > /dev/null 2> error.txt <<-EOF
OFF
EOF

if [ -s error.txt ]
then
    echo "FAILURE. Output detected in STDERR."
    let errors=errors+1
fi

# Again, Look for output with a different permutation of args
./lab4b --scale=C --log=log.txt > /dev/null 2> error.txt <<-EOF
OFF
EOF

if [ -s error.txt ]
then
    echo "FAILURE. Output detected in STDERR."
    let errors=errors+1
fi

# Print total number of errors
if [ $errors -eq 0 ]
then
    echo "Success!"
else
    echo "Failure with $errors errors"
fi

rm error.txt
