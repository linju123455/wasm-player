# ws-player
## 介绍
WASM视频播放器，使用FFMPEG解码SDL2/WebGPU渲染，FFmpeg版本4.4.2。

## 编译
```shell
mkdir build
cd build
emcmake cmake ..
make
```

## 注意事项
如需启用对WebGPU的支持，则需emsdk 3.1.45版本以上，第一次编译工程时，需先关闭对pthread的支持，编译通过之后，再打开对pthread的支持，再编译，这么做的目的是SDL库中依赖的单线程版本的库，如果多线程环境下，SDL编译不过，所以需要先营造单线程环境编译SDL。

## 依赖
需要自行使用emsdk编译ffmpeg。