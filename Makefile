FLAGS= -Wall -pedantic -std=c++11 -fopenmp -I./opencv/include -I./opencv/include/opencv -L./opencv/lib
SOURCES= src/cpp_files
HEADERS= src/header_files

compile: main.o base.o convexHull.o detect.o evaluate.o hardness.o hough.o preprocess.o parameters.o
	g++ $(FLAGS) main.o base.o convexHull.o detect.o evaluate.o hardness.o hough.o preprocess.o parameters.o -o src/main -lpthread -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video

all: links compile libs
clean:
	rm -rf *.o 2>/dev/null;find ./opencv/lib/ -type l -delete
run: 
	chmod +x ./run_it\ -\ all_in_pictures_data.sh;./run_it\ -\ all_in_pictures_data.sh

libs:
	. ./libs.sh

links:
	chmod +x ./opencv/lib/links.sh;cd ./opencv/lib;./links.sh;cd ../../

main.o: src/main.cpp $(HEADERS)/base.h $(HEADERS)/convexHull.h $(HEADERS)/detect.h $(HEADERS)/evaluate.h $(HEADERS)/hardness.h $(HEADERS)/hough.h $(HEADERS)/preprocess.h
	g++ -c $(FLAGS) src/main.cpp -o main.o

base.o: $(SOURCES)/base.cpp $(HEADERS)/base.h $(HEADERS)/hardness.h
	g++ -c $(FLAGS) $(SOURCES)/base.cpp -o base.o

convexHull.o: $(SOURCES)/convexHull.cpp $(HEADERS)/convexHull.h $(HEADERS)/base.h
	g++ -c $(FLAGS) $(SOURCES)/convexHull.cpp -o convexHull.o

detect.o: $(SOURCES)/detect.cpp $(HEADERS)/detect.h $(HEADERS)/base.h
	g++ -c $(FLAGS) $(SOURCES)/detect.cpp -o detect.o

evaluate.o: $(SOURCES)/evaluate.cpp $(HEADERS)/evaluate.h $(HEADERS)/base.h
	g++ -c $(FLAGS) $(SOURCES)/evaluate.cpp -o evaluate.o

hardness.o: $(SOURCES)/hardness.cpp $(HEADERS)/hardness.h $(HEADERS)/base.h
	g++ -c $(FLAGS) $(SOURCES)/hardness.cpp -o hardness.o

hough.o: $(SOURCES)/hough.cpp $(HEADERS)/hough.h $(HEADERS)/base.h
	g++ -c $(FLAGS) $(SOURCES)/hough.cpp -o hough.o

preprocess.o: $(SOURCES)/preprocess.cpp $(HEADERS)/preprocess.h
	g++ -c $(FLAGS) $(SOURCES)/preprocess.cpp -o preprocess.o

parameters.o: $(SOURCES)/parameters.cpp $(HEADERS)/parameters.h $(HEADERS)/base.h $(HEADERS)/convexHull.h $(HEADERS)/detect.h $(HEADERS)/evaluate.h $(HEADERS)/hardness.h $(HEADERS)/hough.h $(HEADERS)/preprocess.h
	g++ -c $(FLAGS) $(SOURCES)/parameters.cpp -o parameters.o
