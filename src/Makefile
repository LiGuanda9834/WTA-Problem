CC = gcc
CXX = g++

all : HelloWorld

HelloWorld: HelloWorld.o
	g++ -lm HelloWorld.o -o HelloWorld 

HelloWorld.o : HelloWorld.cpp
	g++ -c HelloWorld.cpp

clean : 
	rm -f HelloWorld.o HelloWorld