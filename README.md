# Cisc327EC
Connor Onweller and Chris Outhwaite

#How to Run:
* To run the "good" graphs on the sequential program enter 
	"make good<num>"
* To run the "good" graphs on the MPI program enter 
	"make good<num>_mpi
* These "good" graphs are indexed 1 to 3
* To run the larger datasets on the sequential program enter
	"make run_<data set name>"
* To run the larger datasets on the MPI program enter 
	"make run_<data set name>_mpi"
* Our data sets are NY, BAY, and COL
* To produce plots comparing speedup to number of nodes and arcs, enter
	"make plots"

# How our program worked:
* We determined that the most expensive parallelizable action in the sequential
  program was the for loop that when through all of the nodes and found the
  closest one
* We choose to parallelize this via a cyclical distribution across all MPI
  processors, with each proc finding the closest of all of their nodes
* We then used Allgather to send each proc's closest node to all procs
* We then found the closest of all of these nodes and continued as we would
  through the sequential program

#Expiremental results:
(all on 40 processors for MPI)
Data Set | Nodes    | Arcs     | MPI Time (sec) | Seq Time (sec) | Speedup 
---------+----------+----------+----------------+----------------+---------
good1    | 6        | 8        | 0.001108       | 0.001328       | 1.19855 
good2    | 25       | 38       | 0.001616       | 0.002236       | 1.38366
good3    | 35       | 61       | 0.001775       | 0.002484       | 1.39943
NY       | 264346   | 733846   | 19.90940       | 79.53572       | 3.99488
BAY		 | 321270   | 800172   | 79.02903       | 248.6497       | 3.14630
COL 	 | 435666   | 1057066  | 111.6540       | 281.7754       | 2.52364

#Discussion
Our programs improvements in time for the smaller graphs (the "good".gr files),
seemed to be inconsistent. This is likely because our improvement was dependent
on the number of nodes, and these files did not have enough nodes. The
improvement on the larger datasets was far more significant and consistent
because of the amount of nodes. Speedup improvement decreased for the larger
files. This did not seem to make sense, as we would expect speed up to increase
for graphs with more nodes. 
