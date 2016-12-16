#!/bin/bash
if [ -n "$LD_LIBRARY_PATH" ]; then
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/opencv/lib
	echo "Pridano, soucasny obsah: $LD_LIBRARY_PATH";
else
	export LD_LIBRARY_PATH=`pwd`/opencv/lib
	echo "Pridana nova polozka: $LD_LIBRARY_PATH";
fi
