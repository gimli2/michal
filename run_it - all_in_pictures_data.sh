export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/opencv/lib
./src/main -d 0 -t ./out/pictures_data_all_out.txt `ls ./pictures_data/*.JPG`