# ws-server
## 介绍
websocket服务器，负责生产流数据，使用FFMPEG拉流，FFmpeg版本4.4.2。

## 编译
依赖Openssl以及X11（linux平台桌面捕获，用于测试时延），没有这两个库可以先执行
```shell
sudo apt-get install libssl-dev
sudo apt-get install libx11-dev libxext-dev
```
有则忽略

```shell
mkdir build
cd build
cmake ..
make -j4
```

## 注意事项
该工程使用的证书文件为测试文件，没有密码没有期限，如果需要使用websocket Secure server, 证书必须要与web服务器加载的证书一样



