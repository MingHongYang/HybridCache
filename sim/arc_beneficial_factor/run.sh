dram_r=2
dram_w=1
nvram_r=1
nvram_w=1

for file in ~/log/*
do
    filename=$(basename "$file")
    filename="${filename%.*}"

    for dram_index in {1..10}
    do
        echo ${filename}
        dram=$((dram_index * 12000))
        number=./logs/${filename}.output
        stat=./logs/${filename}_${dram}.cnt
        echo $logname
        nvram=$((128000 - dram))
        echo $nvram
        echo $file
        
        ./mine ${file} $number $stat $nvram $dram $dram_r $dram_w $nvram_r $nvram_w
    done
done
