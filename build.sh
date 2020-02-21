#!/bin/bash
set -x
for bin in server gdnative cli_client; do
    cd $bin
    echo Entering directory \`$bin\'
    coddle release || exit 1
    echo Leaving directory \`$bin\'
    cd ..
done
cp gdnative/gdnative.dll client/
