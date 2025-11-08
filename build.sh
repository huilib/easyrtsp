#!/bin/bash

## install dependency
apt update
apt install zlib1g-dev
apt install liblzma-dev

## build dependency
pushd docs
source build_deps.sh
popd 

## build rtsp server
pushd src 
make 
popd 