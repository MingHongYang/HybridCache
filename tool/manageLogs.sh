for dir in *
do
    if [ -d "${dir}" ] ; then
        echo $dir
        cd $dir

        output=base
        output_mine=log
        rm $output
        rm $output_mine
        rm ${output}_t
        rm ${output_mine}_t

        for i in {1..63}
        do
            param=$((i*2000))
            if [ ! -f $output ]; then
                cp base_dram_${param} $output
                cp log_dram_${param} $output_mine
            else
                logname=base_dram_${param}
                logname_mine=log_dram_${param}
                python /home/curtis/sdc/tool/concat.py $output $logname
                python /home/curtis/sdc/tool/concat.py $output_mine $logname_mine
            fi
        done

        awk -F " +" '{for(i=1;i<=NF;i++) a[i,NR]=$i}END{for(i=1;i<=NF;i++) {for(j=1;j<=NR;j++) printf a[i,j] " ";print ""}}' $output > ${output}_t
        awk -F " +" '{for(i=1;i<=NF;i++) a[i,NR]=$i}END{for(i=1;i<=NF;i++) {for(j=1;j<=NR;j++) printf a[i,j] " ";print ""}}' $output_mine > ${output_mine}_t

        cp /home/curtis/sdc/tool/index base_i
        cp /home/curtis/sdc/tool/index log_i

        python /home/curtis/sdc/tool/concat.py base_i ${output}_t
        python /home/curtis/sdc/tool/concat.py log_i ${output_mine}_t

        rm ${output}_t
        rm ${output_mine}_t

        cd ..
    fi
done
