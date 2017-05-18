#!/bin/bash

let errors=0

#check for invalid arguments - return value should be 1
./lab0 --blah > /dev/null 2>STDERR
if [ $? -ne 1 ]
then
    echo "Did not report exit value 1 for unrecognized arguments"
    let errors+=1
fi
if [ ! -s STDERR ]
then
    echo "No error message for unrecognized arguments"
    let errors+=1
else
    echo -n "        "
    cat STDERR
fi

#check for input from non-existent file - return value should be 2
./lab0 --input=nofilewouldhavethisname.txt 2>STDERR
if [ $? -ne 2 ]
then
    echo "Did not report exit value 2 for non-existent input file"
    let errors+=1
fi
if [ ! -s STDERR ]
then
    echo "No error message for invalid input file"
    let errors+=1
else
    echo -n "        "
    cat STDERR
fi

#check for unwriteable output file - return value should be 3
touch unwriteable
chmod 444 unwriteable
./lab0 --output=unwriteable 2>STDERR
if [ $? -ne 3 ]
then
    echo "Did not report exit value 3 for non-writeable output file"
    let errors+=1
fi
if [ ! -s STDERR ]
then
    echo "No error message for non-writeable output file"
    let errors+=1
else
    echo -n "        "
    cat STDERR
fi
rm -f unwriteable

#check for segmentation fault
./lab0 --segfault
if [ $? -ne 139 ]
then
    echo "Didn't cause a segmentation fault"
    let errors+=1
fi

#check for segmentation fault and correct catching mechanism
./lab0 --catch --segfault 2>STDERR
if [ $? -ne 4 ]
then
    echo "Didn't successfully catch the segmentation fault"
    let errors+=1
fi
if [ ! -s STDERR ]
then
    echo "No error message regarding the SIGSEGV catch"
    let errors+=1
else
    echo -n "        "
    cat STDERR
fi

# generate some pattern data (COPIED)
dd if=/dev/urandom of=RANDOM bs=1024 count=1 2> /dev/null

#check normal stdin to stdout
./lab0 < RANDOM > STDOUT
if [ $? -ne 0 ]
then
    echo "Failed to return exit value 0"
    let errors+=1
fi
cmp RANDOM STDOUT > /dev/null
if [ $? -ne 0 ]
then
    echo "Wrong data copied"
    let errors+=1
fi
rm STDOUT

#check input file to normal stdout
./lab0 --input=RANDOM > STDOUT
if [ $? -ne 0 ]
then
    echo "Failed to return exit value 0"
    let errors+=1
fi
cmp RANDOM STDOUT > /dev/null
if [ $? -ne 0 ]
then
    echo "Wrong data copied"
    let errors+=1
fi
rm STDOUT

#check output file creation
./lab0 < RANDOM --output=OUTPUT
if [ $? -ne 0 ]
then
    echo "Failed to return exit value 0"
    let errors+=1
fi
cmp RANDOM OUTPUT > /dev/null
if [ $? -ne 0 ]
then
    echo "Wrong data copied"
    let errors+=1
fi
rm OUTPUT

#check input file to output file
./lab0 --input=RANDOM --output=OUTPUT
if [ $? -ne 0 ]
then
    echo "Failed to return exit value 0"
    let errors+=1
fi
cmp RANDOM OUTPUT > /dev/null
if [ $? -ne 0 ]
then
	echo "Wrong data copied"
	let errors+=1
fi
echo
if [ $errors -eq 0 ]; then
	echo "Smoke test passed"
else
	echo "Smoke test failed with $errors errors"
fi

rm RANDOM
rm OUTPUT
rm STDERR