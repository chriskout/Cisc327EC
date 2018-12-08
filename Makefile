
CC = mpicc

good1: shortest_path examples/good1.gr
	./shortest_path examples/good1.gr 1 2

bad1: shortest_path examples/bad1.gr
	./shortest_path examples/bad1.gr 1 2

run_NY: shortest_path data/USA-road-t.NY.gr
	./shortest_path data/USA-road-t.NY.gr 264346 1

shortest_path : shortest_path.c Makefile
	$(CC) -pedantic -Wall -std=c11 -o shortest_path shortest_path.c

clean:
	rm -rf shortest_path *~
