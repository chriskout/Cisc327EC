# Cisc327EC
For EC project
Expiremental results:
(all on 40 processors for MPI)
Data Set | Nodes    | Arcs     | MPI Time (sec) | Seq Time (sec) | Speedup 
---------+----------+----------+----------------+----------------+---------
good1    | 6        | 8        | 0.000754       | 0.002328       | 3.08753 
good2    | 25       | 38       | 0.001616       | 0.014047       | 8.69245
good3    | 35       | 61       | 0.001775       | 0.008031       | 4.52450
NY       | 264346   | 733846   | 19.90940       | 79.53572       | 3.99488
BAY		 | 321270   | 800172   | 79.02903       | 248.6497       | 3.14630
COL 	 | 435666   | 1057066  | 111.6540       | 281.7754       | 2.52364


Any Data Set bigger then this is too large for git to upload to BeoWolf.
Any Data Set that can be made by ourselves is too small to provide useful
information (Time for 10 Nodes ~= Time for 100 Nodes).
