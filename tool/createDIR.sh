for file in ./
do
    filename=$(basename "$file")
    filename="${filename%.*}"
    echo $filename
    mkdir -p ../$filename
    #outputFile="$filename/$filename"
    #outputFile="$filename"
    outputPath="../$filename/"
    #awk -f 'awkCommand' $file > $outputFile
    mv $file $outputPath
    #awk -f 'awkCheck' $file
done
