# EasyRtsp 

A RtspServer based on C++17.

## How to build
* first, you need to rebuild deps
```bash
cat deps.* > abc
rm -fr deps.* 
mv abc deps.zip
unzip deps.zip
```
* build easyrtsp
```bash
cd src
make
```