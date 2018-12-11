
CC = mpicc

good1: shortest_path examples/good1.gr
	./shortest_path examples/good1.gr 1 2

good1_mpi: shortest_path_mpi examples/good1.gr
	srun -n 10 shortest_path_mpi examples/good1.gr 1 2

bad1: shortest_path examples/bad1.gr
	./shortest_path examples/bad1.gr 1 2

run_NY: shortest_path data/USA-road-t.NY.gr
	./shortest_path data/USA-road-t.NY.gr 264346 1

run_NY_mpi: shortest_path_mpi data/USA-road-t.NY.gr
	srun -n 40 shortest_path_mpi data/USA-road-t.NY.gr 264346 1

shortest_path : shortest_path.c Makefile
	$(CC) -pedantic -Wall -std=c11 -o shortest_path shortest_path.c

shortest_path_mpi : shortest_path_mpi.c Makefile
	$(CC) -pedantic -Wall -std=c11 -o shortest_path_mpi shortest_path_mpi.c

clean:
	rm -rf shortest_path *~
