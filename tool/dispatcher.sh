cd /home/curtis/temp/proj/trace

for dir in *
do
    if [ -d "${dir}" ] ; then
        echo $dir
        cd $dir
        cp /home/curtis/temp/proj/sim/algo/mine .
        cp /home/curtis/temp/proj/sim/base/mine_base .
        for dram_index in {1..10}
        do
            dram=$((dram_index * 12000))
            logname=log_dram_${dram}
            echo $logname
            nvram=$((128000 - dram))
            echo $nvram
            #./mine $dir $logname $nvram $dram
            logname=base_dram_${dram}
            echo $logname
            #./mine_base $dir $logname $nvram $dram
        done
        
        # Process logs

        output=base
        output_mine=log
        rm $output
        rm $output_mine
        rm ${output}_t
        rm ${output_mine}_t

        for i in {1..10}
        do  
            param=$((i*12000))
            if [ ! -f $output ]; then
                cp base_dram_${param} $output
                cp log_dram_${param} $output_mine
            else
                logname=base_dram_${param}
                logname_mine=log_dram_${param}
                python /home/curtis/temp/proj/tool/concat.py $output $logname
                python /home/curtis/temp/proj/tool/concat.py $output_mine $logname_mine
            fi  
        done

        awk -F " +" '{for(i=1;i<=NF;i++) a[i,NR]=$i}END{for(i=1;i<=NF;i++) {for(j=1;j<=NR;j++) printf a[i,j] " ";print ""}}' $output > ${output}_t
        awk -F " +" '{for(i=1;i<=NF;i++) a[i,NR]=$i}END{for(i=1;i<=NF;i++) {for(j=1;j<=NR;j++) printf a[i,j] " ";print ""}}' $output_mine > ${output_mine}_t

        cp /home/curtis/temp/proj/tool/index base_i
        cp /home/curtis/temp/proj/tool/index log_i

        python /home/curtis/temp/proj/tool/concat.py base_i ${output}_t
        python /home/curtis/temp/proj/tool/concat.py log_i ${output_mine}_t

        rm ${output}_t
        rm ${output_mine}_t

        # Generate graph
        # Calculate the average hit rate of read/write
        awk -f '/home/curtis/temp/proj/tool/avgResult' base_i > ${dir}_base_avg
        awk -f '/home/curtis/temp/proj/tool/avgResult' log_i > ${dir}_mine_avg

        # Plot the graph
        gnuplot -e "filename='$dir'" /home/curtis/temp/proj/tool/plot.p

        cd ..
    fi
done
