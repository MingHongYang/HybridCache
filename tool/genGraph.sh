cd /home/curtis/temp/proj/trace

for dir in *
do
    if [ -d "${dir}" ] ; then
        echo $dir
        cd $dir
       
        # Generating graphs

        # Calculate the average hit rate of read/write
        awk -f '/home/curtis/temp/proj/tool/avgResult' base_i > ${dir}_base_avg
        awk -f '/home/curtis/temp/proj/tool/avgResult' log_i > ${dir}_mine_avg

        # Plot the graph
        gnuplot -e "filename='$dir'" /home/curtis/temp/proj/tool/plot.p

        cd ..
    fi
done
