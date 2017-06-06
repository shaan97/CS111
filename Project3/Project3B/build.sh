#!/bin/bash

g++ -std=c++11 -static-libstdc++ lab3b.cpp -o lab3b
if [ $? -ne 0 ]
then
    g++ -std=c++11 lab3b.cpp -o lab3b
fi
