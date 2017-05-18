#!/bin/bash

# NAME: SHAAN MATHUR
# UID: 904606576
# EMAIL: SHAANKARANMATHUR@GMAIL.COM

let errors=0

# Check to see if copying is successful with input file
head -5000 /dev/urandom > rand.txt

./lab0 --input="rand.txt" > result.txt
if [ $? -ne 0 ]
then
    echo "ERROR: Exit status for program was not 0"
    let errors+=1
fi
diff rand.txt result.txt
rm result.txt

# Check to see if copying is successful with STDIN
./lab0 --output="result.txt" < rand.txt
if [ $? -ne 0 ]
then
    echo "ERROR: Exit status for program was not 0"
    let errors+=1
fi
diff rand.txt result.txt
rm result.txt
rm rand.txt

# Check to see for failure with bad argument
./lab0 --badarg 2> /dev/null
if [ $? -ne 1 ]
then
    echo "ERROR: Exit status for bad argument should be 1"
    let errors+= 1
fi

# Check to see for failure with bad input file
./lab0 --input="no_input_file.txt" 2> /dev/null
if [ $? -ne 2 ]
then
    echo "ERROR: Exit status for bad input file should be 2"
    let errors+=1
fi

# Check to see for failure with bad output file
touch bad_output.txt
chmod -w bad_output.txt
./lab0 --output="bad_output.txt" 2> /dev/null
if [ $? -ne 3 ]
then
    echo "ERROR: Exit status for bad output file should be 3"
    let errors+=1
fi
chmod +w bad_output.txt
rm bad_output.txt

# Check to see for failure due to segmentation fault
./lab0 --segfault 2> /dev/null
if [ $? -ne 139 ]
then
    echo "ERROR: Exit status for segfault should be 139"
    let errors+=1
fi

# Check to see for failure after segmentation fault handling
./lab0 --segfault --catch 2> /dev/null
if [ $? -ne 4 ]
then
    echo "ERROR: Exit status for caught segfault should be 4"
    let errors+=1
fi

echo "Total Errors: $errors"
