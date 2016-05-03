export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/opencv/lib
cd ../
./src/main -d 0 -i ./pictures_data/lists/DR6-TD.txt -t ./out/DR6-TD_out.txt
./src/main -d 0 -i ./pictures_data/lists/DR6-RD.txt -t ./out/DR6-RD_out.txt
./src/main -d 0 -i ./pictures_data/lists/DAR6-TD.txt -t ./out/DAR6-TD_out.txt
./src/main -d 0 -i ./pictures_data/lists/DAR6-RD.txt -t ./out/DAR6-RD_out.txt