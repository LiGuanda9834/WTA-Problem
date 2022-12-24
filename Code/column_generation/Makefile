CC 		= g++
CC_o 	= -o
CC_c 	= -c


CURDIR := $(SHELL pwd)

HEADER 	= clique_graph.h

LIBOBJ 	= clique_graph.o
MAINOBJ = main.o

MAINBIN = MCDFR_col_ger_subProblem

all : ${LIBOBJ} ${MAINOBJ}
	@-echo "linking..."
	g++ -fopenmp -g $^ -o ${MAINBIN}

${LIBOBJ} : %.o : %.cpp %.h
	@-echo "compling "$< "..."
	g++ -fopenmp -g -c $< -o $@

${MAINOBJ} : %.o : %.cpp
	@-echo "compiling "$< "..."
	g++ -fopenmp -g -c $< -o $@

.PHONY:clean
clean:
	@-rm -rf ${LIBOBJ} 
	@-rm -rf ${MAINOBJ} 
	@-rm -rf ${MAINBIN}
