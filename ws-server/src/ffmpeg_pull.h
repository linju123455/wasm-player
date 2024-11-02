/*================================================================
*   Copyright (c) 2024, Inc. All Rights Reserved.
*   
*   @file：ffmpeg_pull.h
*   @author：linju
*   @email：15013144713@163.com
*   @date ：2024-05-20
*   @berief： ffmpeg puller
*
================================================================*/
#pragma once
#include <iostream>
#include <map>
#include <mutex>
#include <memory>

extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>

#include "wrapper.h"

class FFmpegPuller
{
public:
    FFmpegPuller();
    virtual ~FFmpegPuller();

    bool OpenRtspStream(const std::string &rtsp_url);

    void WsStop(websocketpp::connection_hdl hdl);

    void PullStream();

    void SetHandlers(void* ws_server, websocketpp::connection_hdl hdl, int ws_server_type);

    void SendCodecInfo(websocketpp::connection_hdl hdl);

private:
    struct WsHdlInfo {
        void* ws_server;
        std::shared_ptr<void> wshdl;
        int ws_server_type; // 0: ws, 1:wss
    };

private:
    AVFormatContext* pFormatCtx = NULL; // 保存视频流的信息的上下文
    AVPacket *h264Pack = NULL; // 保存解码之前的数据
    AVCodecID video_codec_id_;
    int width_ = 0;
    int height_ = 0;
    uint8_t* extradata_ = nullptr;
    size_t extradata_size_ = 0;

    int32_t video_index = -1; // 视频下标
    int32_t audio_index = -1; // 音频下标

    bool first = true;
    bool wasm_start_ = false;
    bool pull_test_quic_ = false;
    std::map<void*, WsHdlInfo> ws_infos_;
    std::mutex mutex_;
};