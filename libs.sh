if [ -n "$LD_LIBRARY_PATH" ]; then
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/opencv/lib
else
    export LD_LIBRARY_PATH=`pwd`/opencv/lib
fi