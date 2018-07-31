cp /home/curtis/sdc/sim/analyze/a.out .

for file in ./*.f
do
    filename=$(basename "$file")
    filename="${filename%.*}"
    echo $filename
    #mkdir -p $filename
    outputFile="trace/$filename.out"
    writeOut="trace/$filename.write"
    filePath="$filename/$filename"
    #awk -f 'awkCommand' $file > $outputFile
    #mv $file $filename
    #awk -f 'awkCheck' $file
    ./a.out $filePath $outputFile $writeOut
done
