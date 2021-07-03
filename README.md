# RTS2-Synthesizer
## Getting started
### Prerequisites
* SoundIO
* Cmake
* pulseaudio
### Install Pre
``` bash
$ sudo apt-get install libsoundio-dev cmake pulseaudio
```
### How to build and run 
```bash
$ mkdir build && cd build
$ cmake ..
$ make
$ sudo pulseaudio -D
$ sudo ./RTS_Synthesizer /dev/input/event4
```
Add right device path for keyboard as argument
