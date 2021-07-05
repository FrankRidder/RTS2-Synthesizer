# RTS2-Synthesizer
## Getting started
### Prerequisites
* Cmake
```bash
$ sudo apt install cmake
```
OpenAL
```bash
$ sudo apt-get install -y openal-info
```
If CMake can not find OpenAL try:
```bash
$ apt-cache search openal
```
or
```bash
$ sudo apt-get install lobopenal-dev
```

### How to build and run 
```bash
$ mkdir build && cd build
$ cmake ..
$ make
$ sudo ./RTS_Synthesizer /dev/input/event4
```
Add right device path for keyboard as argument
