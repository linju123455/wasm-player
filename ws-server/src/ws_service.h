/*================================================================
*   Copyright (c) 2024, Inc. All Rights Reserved.
*   
*   @file：ws_service.h
*   @author：linju
*   @email：15013144713@163.com
*   @date ：2024-05-20
*   @berief： websocket service code
*
================================================================*/
#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <memory>
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include "singleton.h"
#include "ffmpeg_pull.h"

typedef websocketpp::server<websocketpp::config::asio> WsServer;
class WebSocketSvr
{
public:
    WebSocketSvr();
    virtual ~WebSocketSvr();

public:
    int Run(uint16_t port, int thread_count);

    void Stop();

protected:
    void HandleOpen(websocketpp::connection_hdl hdl);

    bool HandleValidate(websocketpp::connection_hdl hdl);

    void HandleClose(websocketpp::connection_hdl hdl);

    void HandleMsg(websocketpp::connection_hdl hdl, WsServer::message_ptr msg);

private:
    WsServer server_handle_;
    std::vector<std::shared_ptr<std::thread>> threads_;
};