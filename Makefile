
CC = mpicc

good1: shortest_path examples/good1.gr
	./shortest_path examples/good1.gr 1 2

good2: shortest_path examples/good2.gr
	./shortest_path examples/good2.gr 1 25

good3: shortest_path examples/good3.gr
	./shortest_path examples/good3.gr 1 35

good1_mpi: shortest_path_mpi examples/good1.gr
	srun -n 40 shortest_path_mpi examples/good1.gr 1 2

good2_mpi: shortest_path_mpi examples/good2.gr
	srun -n 40 shortest_path_mpi examples/good2.gr 1 25

good3_mpi: shortest_path_mpi examples/good3.gr
	srun -n 40 shortest_path_mpi examples/good3.gr 1 35

bad1: shortest_path examples/bad1.gr
	./shortest_path examples/bad1.gr 1 2

run_NY: shortest_path data/USA-road-t.NY.gr
	srun shortest_path data/USA-road-t.NY.gr 264346 1

run_NY_mpi: shortest_path_mpi data/USA-road-t.NY.gr
	srun -n 40 shortest_path_mpi data/USA-road-t.NY.gr 264346 1

shortest_path : shortest_path.c Makefile
	$(CC) -pedantic -Wall -std=c11 -o shortest_path shortest_path.c

shortest_path_mpi : shortest_path_mpi.c Makefile
	$(CC) -pedantic -Wall -std=c11 -o shortest_path_mpi shortest_path_mpi.c

run_COL_mpi: shortest_path_mpi data/USA-road-t.COL.gr
	srun -n 40 shortest_path_mpi data/USA-road-t.COL.gr 435665 1

run_COL: shortest_path data/USA-road-t.COL.gr
	srun shortest_path data/USA-road-t.COL.gr 435665 1

run_BAY_mpi: shortest_path_mpi data/USA-road-t.BAY.gr
	srun -n 40 shortest_path_mpi data/USA-road-t.BAY.gr 321270 1

run_BAY: shortest_path data/USA-road-t.BAY.gr
	srun shortest_path data/USA-road-t.BAY.gr 321270 1

arc_graph: speedup-per-arc.gnu arcs.dat
	gnuplot "speedup-per-arc.gnu"

node_graph: speedup-per-node.gnu nodes.dat
	gnuplot "speedup-per-node.gnu"

plots: node_graph arc_graph

clean:
	rm -rf shortest_path *~
