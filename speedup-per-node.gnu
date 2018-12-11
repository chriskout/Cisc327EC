col = 2
row = 0
stats "nodes.dat" every ::row::row using col nooutput
seq_time = STATS_min
set terminal pdf  # size 4, 4
set output "mpi-speed-up-over-nodes.pdf"
# unset log
# unset label
# set key vertical top left 
set xlabel left "Numer of nodes"
set ylabel left "time (seconds)"
set y2label right "Speedup"
set xr [0:1200000]
set yr [0:5]
#plot  "arcs.dat" using 1:(seq_time/($2)) title 'Speedup' with linespoints axes x1y2
plot  "nodes.dat" using 1:2 title 'mpi vs seq' with linespoints
