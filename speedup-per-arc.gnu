col = 2
row = 0
stats "arcs.dat" every ::row::row using col nooutput
seq_time = STATS_min
set terminal pdf  # size 4, 4
# set tmargin at screen 0.9
# set lmargin at screen 0.1
# set rmargin at screen -0.1
# set bmargin at screen 0.1
# set size ratio 1.2
set output "mpi-speed-up-over-arcs.pdf"
# unset log
# unset label
# set key vertical top left 
set xlabel left "Numer of arcs"
set ylabel left "time (seconds)"
set y2label right "Speedup"
# set logscale y
# set format y "10^{%T}"
# set xtics 1, 1, 10
set xr [0:1500000]
set yr [0:80]
set y2r [0:10]
set ytics 0, 10, 80 nomirror
set y2tics 0, 1, 10 nomirror
plot  "arcs.dat" using 1:(seq_time/($2)) title 'Speedup' with linespoints axes x1y2
