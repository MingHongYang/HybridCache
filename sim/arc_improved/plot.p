# Gnuplot script file for plotting data
# This file is called   plot.p

partitionSize = sprintf("./fig/%s_partition.png", filename)
targetSize = sprintf("./fig/%s_target.png", filename)
compare_w = sprintf("./fig/%s_comp_w.png", filename)
compare_r = sprintf("./fig/%s_comp_r.png", filename)

set terminal png
set output partitionSize
set autoscale                          # scale axes automatically
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title "Size of each Partition"
set xlabel "Time"
plot \
filename using 7 title 'Write Frequency' with lines , \
filename using 8 title 'Write Recency' with lines , \
filename using 9 title 'Read Frequency' with lines , \
filename using 10 title 'Read Recency' with lines

set output targetSize
set title "Target Size"
plot \
filename using 1 title 'Write' with lines , \
filename using 2 title 'W_F' with lines , \
filename using 3 title 'W_R' with lines , \
filename using 4 title 'Read' with lines , \
filename using 5 title 'R_F' with lines , \
filename using 6 title 'R_R' with lines

set output compare_w
set title "Compare Write"
plot \
filename using 2 title 'W_F' with lines , \
filename using 3 title 'W_R' with lines , \
filename using 7 title 'Write Frequency' with lines , \
filename using 8 title 'Write Recency' with lines

set output compare_r
set title "Compare Read"
plot \
filename using 5 title 'R_F' with lines , \
filename using 6 title 'R_R' with lines, \
filename using 9 title 'Read Frequency' with lines , \
filename using 10 title 'Read Recency' with lines
