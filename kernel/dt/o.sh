#!/bin/bash

KDIR=/home/slava/my/br/buildroot/output/build/linux-custom

gcc -E -nostdinc -I$KDIR/include -x assembler-with-cpp -undef -imacros 3308.h -o $1.tmp  $1 
./dtc -@ -I dts -O dtb -o ${1%%.*}.dtbo $1.tmp
