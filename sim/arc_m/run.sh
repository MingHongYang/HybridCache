
for file in ~/log/*
do
    filename=$(basename "$file")
    filename="${filename%.*}"

    for dram_index in {1..10}
    do
        echo ${filename}
        dram=$((dram_index * 12000))
        number=./log/${filename}.output
        stat=./log/${filename}.size
        echo $logname
        nvram=$((128000 - dram))
        echo $nvram
        echo $file
        
        ./mine ${file} $number $stat $nvram $dram
    done
done
