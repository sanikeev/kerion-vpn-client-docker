#!/bin/bash

echo "Removing all binaries..."
rm ./src/linux/*.o
rm ./*.so
rm ./example/src/linux/*.o
rm ./kvnet_example

echo "Compiling dynamic library..."
gcc -Wall -I./src/include -o ./src/linux/kvnet.o -c ./src/linux/kvnet.c

echo "Linking dynamic library..."
gcc -Wall -shared -o ./libkvnet.so ./src/linux/kvnet.o -lpthread

echo "Compiling executable..."
c++ -Wall -I./src/include -o ./example/src/linux/main.o -c ./example/src/linux/main.cpp

echo "Linking executable..."
c++ -Wall ./example/src/linux/main.o -o ./kvnet_example ./libkvnet.so -lpthread

exit
