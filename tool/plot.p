# Gnuplot script file for plotting data
# This file is called   plot.p

avgMine  = sprintf("%s_mine_avg", filename)
avgBase = sprintf("%s_base_avg", filename)

rHit = sprintf("../../fig/%s_r_hit.png", filename)
wHit = sprintf("../../fig/%s_w_hit.png", filename)
hitDRAM = sprintf("../../fig/%s_hit_dram.png", filename)
hitNVRAM = sprintf("../../fig/%s_hit_nvram.png", filename)

set terminal png
set output rHit
set autoscale                          # scale axes automatically
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
# set yrange [0:1]
set title "Read Hit Ratio"
set xlabel "DRAM to NVRAM Ratio (Total 128K)"
plot \
avgMine using 1:3 title 'Our' with linespoints , \
avgBase using 1:3 title 'Base' with linespoints

set output wHit
set title "Write Hit Ratio"
plot \
avgMine using 1:2 title 'Our' with linespoints , \
avgBase using 1:2 title 'Base' with linespoints

set output hitDRAM
set title "Page Hit on DRAM"
plot \
avgMine using 1:6 title 'Our_R' with linespoints , \
avgMine using 1:4 title 'Our_W' with linespoints , \
avgBase using 1:6 title 'Base_R' with linespoints , \
avgBase using 1:4 title 'Base_W' with linespoints

set output hitNVRAM
set title "Page Hit on NVRAM"
plot \
avgMine using 1:7 title 'Our_R' with linespoints , \
avgMine using 1:5 title 'Our_W' with linespoints , \
avgBase using 1:7 title 'Base_R' with linespoints , \
avgBase using 1:5 title 'Base_W' with linespoints

