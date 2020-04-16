
for file in ~/log/*
do
    filename=$(basename "$file")
    filename="${filename%.*}"

    echo $filename

    #for dram_index in {1..10}
    #do
    #    echo ${filename}
    #    dram=$((dram_index * 12000))
    number=./logs/12_116.output
    #    stat=./logs/${filename}.size
    #    echo $logname
    #    nvram=$((128000 - dram))
    #    echo $nvram
    #    echo $file
        
    ./mine ${file} $number 12000 116000
    #done
done
