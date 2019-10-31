
for file in ~/log/*
do
    filename=$(basename "$file")
    filename="${filename%.*}"

    for dram_index in {1..10}
    do
        echo ${filename}
        dram=$((dram_index * 12000))
        logname=./log/${filename}_dram_${dram}
        echo $logname
        #./mine ${file} $logname $nvram $dram

        awk -f 'avgResult' $logname >> ${filename}_avg
        awk -f 'stats' $logname >> ${filename}_stat
        #awk -f '/' log_i > ${dir}_mine_avg
    done
done
