for dir in *
do
    if [ -d "${dir}" ] ; then
        cp -r ${dir} ../
    fi
done
