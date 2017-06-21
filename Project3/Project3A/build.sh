#!/bin/bash

g++ -std=c++11 -static-libstdc++ EXT2_info.cpp lab3a.cpp -o lab3a -lm
if [ $? -ne 0 ]
then
    g++ -std=c++11 EXT2_info.cpp lab3a.cpp -o lab3a -lm
fi

