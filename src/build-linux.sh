#!/bin/bash
g++ -std=c++11 -I../opencv/include -I../opencv/incude/opencv2 -I../opencv/include/opencv -L../opencv/lib main.cpp ./cpp_files/base.cpp ./cpp_files/convexHull.cpp ./cpp_files/detect.cpp ./cpp_files/evaluate.cpp ./cpp_files/hardness.cpp ./cpp_files/hough.cpp ./cpp_files/preprocess.cpp ./cpp_files/parameters.cpp -o michal-lin -Wall -pedantic -fopenmp -lpthread -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_video -lopencv_imgcodecs -lopencv_videoio

#win
#g++ -std=c++11 -I../opencv/include -I../opencv/incude/opencv2 -I../opencv/include/opencv -L../opencv/lib_win -L../opencv/lib_win_3rdparty main.cpp -o main cpp_files/base.cpp cpp_files/detect.cpp cpp_files/hardness.cpp cpp_files/parameters.cpp cpp_files/convexHull.cpp cpp_files/evaluate.cpp cpp_files/hough.cpp cpp_files/preprocess.cpp -lstdc++ -lopencv_world310 -lopencv_ts310 -lIlmImf -llibjasper -llibjpeg -llibpng -llibtiff -llibwebp -lzlib -lgdi32 -lcomdlg32

