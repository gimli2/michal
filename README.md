# MICHAL v2
## MICro Hardness AnaLysis

### Building & installation:
The program have been tested for Windows 7 and Linux Mint 15 and 17.

**For Windows:**
  - Add absolute path to MinGW\bin to windows Environment Variables - PATH. (This step isn't needed if there already is an installation of gcc compiler 4.9.X or newer.)
  - Compile by executing src/build-windows.bat

**For linux:**

Requirements:
  * gcc compiler 4.8.4 or newer.
  * development packages: libwebp-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev zlib1g-dev

Compile:
  - Run makefile (make all).
  - Add path to LD_LIBRARY_PATH variable to opencv/lib manually or by executing ". ./libs.sh" or alternatively: run src/buil-linux.sh

### Run program:
Run src/michal-win.exe or src/michal-lin (built by export LD_LIBRARY_PATH=`pwd`/opencv/lib) or main (built by make).

### Results
Image with detected puncture:
![Image with detected puncture](https://raw.githubusercontent.com/gimli2/michal/master/text/evaluated_image.jpg)
