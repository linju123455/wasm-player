# wasm-demo
## 文件介绍
以下代码均运行在linux平台
- any-test: 前端代码，调用wasm模块，负责建立websocket连接，灌数据给wasm；
- ws-player: wasm视频播放器代码，实现web上视频的解码以及渲染操作;
- ws-server: websocket服务端代码，实现拉流服务器，以及websocket的传输链路。

## 本地部署
### 前端
名为any-test的代码，送入任何一个web服务器，使用nodejs+npm举例：
执行
```shell
npm run build
npm run serve
```
即可在本地访问：`http://localhost:8080/test-ws-player`;

接着将ws-player编译完成生成三个文件，将三个文件放入any-test/public/js下，将名为ws-player.js的文件放入any-test/src/wasm下;
至此前端工作准备完成。

### 服务端
ws-server进入代码，改变为自己想要拉取的流地址，编译完成生成服务端执行程序，运行; 接着在前端页面点击打开按钮，不出意外，你应该可以在一小会延迟之后看到你想看到的流。

## 非本地部署
与本地部署不同的是，非本地部署需要考虑跨源和安全性的问题，所以需要使用到HTTPS服务，否则无法跨源访问;
有以下两种方案选择

- nginx做反向代理，前端的HTTPS请求全部由nginx转成HTTP请求代理到服务端。
- websocket服务端加上允许跨源访问的头，这样前端也可以直接HTTPS通信。