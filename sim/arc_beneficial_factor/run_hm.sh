dram_r=4
dram_w=-1
nvram_r=2
nvram_w=1

for file in ~/log/hm_1
do
    filename=$(basename "$file")
    filename="${filename%.*}"

    for dram_index in {1..10}
    do
        echo ${filename}
        dram=$((dram_index * 12000))
        number=./logs_2/${filename}.output
        stat=./logs_2/${filename}_${dram}.cnt
        echo $logname
        nvram=$((128000 - dram))
        echo $nvram
        echo $file
        
        ./mine ${file} $number $stat $nvram $dram $dram_r $dram_w $nvram_r $nvram_w
    done
done
