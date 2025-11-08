#!/bin/bash

# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# create output directory
CCP_DIR=${SCRIPT_DIR}/..
OUTPUT=${CCP_DIR}/deps
mkdir -p ${OUTPUT}

## compile ffmpeg
FFMPEG_OUTPUT=${OUTPUT}/ffmpeg
sudo apt install yasm -y
sudo apt install nasm -y
sudo apt install pkg-config -y
unzip ffmpeg4.2.7.zip
pushd ffmpeg4.2.7
    ./configure --prefix=${FFMPEG_OUTPUT}
    make -j${nproc}
    make install
popd 

## compile huicpp
unzip v7s0.zip
pushd v7s0
    ./install.sh
    pushd deps/linux64
        cp -r huicpp ${OUTPUT}/huicpp
        cp -r json ${OUTPUT}/json
        cp -r openssl ${OUTPUT}/openssl
    popd 
popd 

