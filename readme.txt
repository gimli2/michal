The programm have been tested for Windows 7 and Linux Mint 15 and 17.


Installation under Windows:
1)
Add absolute path to MinGW\bin to windows Environment Variables - PATH.
This step isn't needed if there already is an installation of gcc compiler 4.9.X or newer.
2)
Compile by executing src/build-windows.bat

Run programm:
Run src/main.exe by the command-line interpreter or using testing .bat files.


Installation under linux:
requirements:
gcc compiler 4.9.X or newer.
development packages: libwebp-dev, libjpeg-dev, libpng-dev, libtiff-dev, libjasper-dev, zlib1g-dev

1)
Run makefile (make all).
2)
Add path to LD_LIBRARY_PATH variable to opencv/lib manually or by executing ". ./libs.sh".

Run programm:
Run src/main by the command-line interpreter or using testing .sh files.