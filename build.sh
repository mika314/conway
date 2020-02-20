#!/bin/bash
set -x
for bin in server gdnative; do
    cd $bin
    echo Entering directory \`$bin\'
    coddle debug
    echo Leaving directory \`$bin\'
    cd ..
done
cp gdnative/gdnative.dll client/
