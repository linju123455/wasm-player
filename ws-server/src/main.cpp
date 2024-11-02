// create by linju 2024.05.20
#include <thread>
#include <future>
#include <iostream>
#include "ffmpeg_pull.h"
#include "wss_service.h"
#include "ws_service.h"
#include "singleton.h"

void ProgExit(int signo)
{
    std::cout << "come in prog exit" << std::endl;
    Singleton<WebSocketSvr>::Instance().Stop();
    Singleton<WebSocketSecureSvr>::Instance().Stop();
    exit(0);
}

void errorCb(int signo)
{
    std::cout << "come in errorCb" << std::endl;
    exit(-1);
}

int main(void) 
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGKILL, ProgExit);
    signal(SIGINT, ProgExit);
    signal(SIGTERM, ProgExit);
    signal(SIGSEGV, errorCb);
    signal(SIGABRT, errorCb);

    std::string rtsp_url = "rtsp://172.16.41.55/live/4K";
    Singleton<FFmpegPuller>::Instance().OpenRtspStream(rtsp_url);
    
    std::future<void> result = std::async(std::launch::async, [](){
        Singleton<FFmpegPuller>::Instance().PullStream();
    });

    Singleton<WebSocketSvr>::Instance().Run(20080, 4);

    Singleton<WebSocketSecureSvr>::Instance().SetTLSFiles("/home/linju/linju_workspace/cert.pem", "/home/linju/linju_workspace/key.pem");
    Singleton<WebSocketSecureSvr>::Instance().Run(20443, 4);

    while (1) {
       std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}