# EasyRtsp 

A high-performance, low-memory RtspServer based on C++17. 

EasyRtsp supports:
* H264/ACC video stream;
* UDP over RTP/RTSP;
* TCP over RTP/RTSP;
* unicast and multicast;
* Single stream support (one video stream + one audio stream);
* Frame buffer tunning for streaming server (default is 10 frames buffering (audio/video));

## How to build
```bash
sudo ./build.sh
```