CC = g++
CFLAGS = -ggdb -std=c++2a

all: sudoku_solver.cpp
	$(CC) $(CFLAGS) sudoku_solver.cpp -o sudoku_solver

clean: 
	rm *.o *.exe