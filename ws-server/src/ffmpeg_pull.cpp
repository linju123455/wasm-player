#include "ffmpeg_pull.h"

FFmpegPuller::FFmpegPuller()
{

}

FFmpegPuller::~FFmpegPuller()
{

}

bool FFmpegPuller::OpenRtspStream(const std::string &rtsp_url)
{
    int ret = -1;
    av_register_all();
    avformat_network_init();

    pFormatCtx = avformat_alloc_context(); 
    if (!pFormatCtx) return false;

    AVDictionary *options = NULL; 
    av_dict_set(&options, "buffer_size", "1048576", 0);
    av_dict_set(&options, "rtsp_transport", "tcp", 0); 
    av_dict_set(&options, "max_delay", "500000", 0);
    av_dict_set(&options, "stimeout", "10000000", 0);
    av_dict_set(&options, "max_analyze_duration", "100000", 0);

    ret = avformat_open_input(&pFormatCtx, rtsp_url.c_str(), NULL, &options);
    if (ret != 0) {
        std::cout << "Couldn't open input stream:" << rtsp_url.c_str()<<std::endl;
        return false;
    }

    ret = avformat_find_stream_info(pFormatCtx, NULL);
    if (ret != 0) {
        std::cout << "Couldn't find stream information.\n";
        return false;
    }

    std::cout<<"--------------input stream info start-----------------"<<std::endl;
    av_dump_format(pFormatCtx, 0, rtsp_url.c_str(), 0); // 打印输入流信息
    std::cout<<"--------------input stream info end-----------------"<<std::endl;

    for (int a = 0; a < pFormatCtx->nb_streams; a++) {
        std::cout<<"codec_type is:"<<pFormatCtx->streams[a]->codec->codec_type<<std::endl;
        if (pFormatCtx->streams[a]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (pFormatCtx->streams[a]->codec->extradata) {
                std::cout << "extradata_size is : " << pFormatCtx->streams[a]->codec->extradata_size << std::endl;
                extradata_ = new uint8_t[pFormatCtx->streams[a]->codec->extradata_size];
                memcpy(extradata_, pFormatCtx->streams[a]->codec->extradata, pFormatCtx->streams[a]->codec->extradata_size);
                extradata_size_ = pFormatCtx->streams[a]->codec->extradata_size;
                // for (int i = 0; i < extradata_size_; i++)
                // {
                //     printf("%02X ", extradata_[i]);
                //     if((i+1)%16==0)
                //     {
                //         printf("\n");
                //     }
                // }
            }
            std::cout<<"video index = "<<a<<std::endl;
            video_index = a;
            video_codec_id_ = pFormatCtx->streams[a]->codec->codec_id;
            std::cout<<"video codec id is : "<<video_codec_id_<<std::endl;
            width_ = pFormatCtx->streams[a]->codec->width;
            height_ = pFormatCtx->streams[a]->codec->height;
            std::cout << "video width is : " << width_ << " height is : " << height_ << std::endl;
            break;
        }
        if (pFormatCtx->streams[a]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_index = a;
        }
    }

    if (video_index == -1)
    {
        std::cout << "Didn't find a video stream.\n";
        return false;
    }

    return true;
}

void FFmpegPuller::WsStop(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lk(mutex_);
    if (hdl.expired()) {
        std::cout << "Stop hdl is expired" << std::endl;
        return;
    }
    auto conn_ptr = hdl.lock().get();
    if (!ws_infos_.count(conn_ptr)) {
        std::cout << "ws_infos_ is dont find conn" << std::endl;
        return;
    }
    ws_infos_.erase(conn_ptr);
}

void FFmpegPuller::PullStream() {
    std::cout << "come in PullStream " << std::endl;
#if 0
    // Initialize FFmpeg library
    avcodec_register_all();
    av_register_all();

    // Open X11 display
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open X11 display" << std::endl;
        return;
    }

    // Get the root window
    Window root = DefaultRootWindow(display);
    XWindowAttributes window_attributes;
    XGetWindowAttributes(display, root, &window_attributes);
    int width = window_attributes.width;
    int height = window_attributes.height;

    // Find the H.265 encoder
    AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_HEVC);
    if (!codec) {
        std::cerr << "Codec not found" << std::endl;
        return;
    }

    // Allocate codec context
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    codec_ctx->width = width;
    codec_ctx->height = height;
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    codec_ctx->time_base = {1, 60};
    codec_ctx->framerate = {60, 1};
    codec_ctx->bit_rate = 1000000; // Increase bit rate for better quality
    codec_ctx->gop_size = 10; // Set group of pictures size
    codec_ctx->max_b_frames = 0; // Disable B-frames for lower latency
    codec_ctx->rc_buffer_size = 2000000; // Set buffer size to 2MB
    codec_ctx->rc_max_rate = 2000000; // Set max rate to 2MB
    codec_ctx->rc_min_rate = 2000000; // Set min rate to 2MB

    // Open the codec
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "preset", "ultrafast", 0); // Set encoding preset to ultrafast
    av_dict_set(&opts, "tune", "zerolatency", 0); // Tune for zero latency
    if (avcodec_open2(codec_ctx, codec, &opts) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        return;
    }

    // Allocate frame
    AVFrame* frame = av_frame_alloc();
    frame->format = codec_ctx->pix_fmt;
    frame->width = codec_ctx->width;
    frame->height = codec_ctx->height;
    av_frame_get_buffer(frame, 32);

    int pts = 0;
#endif

    // Allocate packet
    h264Pack = (AVPacket*)av_malloc(sizeof(AVPacket));
    av_init_packet(h264Pack);
    while(!pull_test_quic_) {
        {
            std::lock_guard<std::mutex> lk(mutex_);
            if (pull_test_quic_) {
                std::cout << "pull thread quic" << std::endl;
                break;
            }
        }

#if 0
        XImage* image = XGetImage(display, root, 0, 0, width, height, AllPlanes, ZPixmap);
        if (!image) {
            std::cerr << "Failed to capture image" << std::endl;
            break;
        }

        // Copy the image data to frame buffer
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                unsigned long pixel = XGetPixel(image, x, y);
                uint8_t r = (pixel & image->red_mask) >> 16;
                uint8_t g = (pixel & image->green_mask) >> 8;
                uint8_t b = (pixel & image->blue_mask);

                // Convert RGB to YUV420P
                frame->data[0][y * frame->linesize[0] + x] = (0.299 * r + 0.587 * g + 0.114 * b);  // Y
                if (y % 2 == 0 && x % 2 == 0) {
                    frame->data[1][(y / 2) * frame->linesize[1] + (x / 2)] = (-0.168736 * r - 0.331264 * g + 0.5 * b + 128);  // U
                    frame->data[2][(y / 2) * frame->linesize[2] + (x / 2)] = (0.5 * r - 0.418688 * g - 0.081312 * b + 128);  // V
                }
            }
        }

        frame->pts = pts;
        pts += 1;
        // Encode the frame
        if (avcodec_send_frame(codec_ctx, frame) >= 0) {
            if (avcodec_receive_packet(codec_ctx, h264Pack) >= 0) {
                // Output the encoded packet (H.265 data)
                if (!wasm_start_) {
                    av_packet_unref(h264Pack);
                    continue;
                }
                stAVPacketInfo pkt;
                pkt.data.data = h264Pack->data;
                pkt.data.len = h264Pack->size;
                std::shared_ptr<Wrapper> wrapper = std::make_shared<Wrapper>();
                std::string data;
                wrapper->WrapAVPacket((stAVPacketInfo*)&pkt, data);

                {
                    std::lock_guard<std::mutex> lk(mutex_);
                    for (auto ws_info : ws_infos_) {
                        websocketpp::lib::error_code ec;
                        if (ws_info.second.ws_server_type == 1) {
                            websocketpp::server<websocketpp::config::asio_tls>* ws_server = (websocketpp::server<websocketpp::config::asio_tls>*)ws_info.second.ws_server;
                            ws_server->send(websocketpp::connection_hdl(ws_info.second.wshdl), data, websocketpp::frame::opcode::binary, ec);
                        }
                        else {
                            websocketpp::server<websocketpp::config::asio>* ws_server = (websocketpp::server<websocketpp::config::asio>*)ws_info.second.ws_server;
                            ws_server->send(websocketpp::connection_hdl(ws_info.second.wshdl), data, websocketpp::frame::opcode::binary, ec);
                        }
                        if (ec) {
                            std::cout << "send AVPacket failed: " << ec.message() << std::endl;
                        }
                    }
                }
                av_packet_unref(h264Pack);
                // frame_count++;
            }
        }

        XDestroyImage(image);
#endif

        // 接受原始数据
        if (av_read_frame(pFormatCtx,h264Pack) < 0) {
            std::cout<<"get h264 data failed"<<std::endl;
            av_packet_unref(h264Pack); //释放packet
            continue;
        }

        if (h264Pack->stream_index != video_index) {
            // std::cout<<"stream idx is not video index" << h264Pack->stream_index <<std::endl;
            av_packet_unref(h264Pack);
            continue;
        }

        if (!wasm_start_) {
            av_packet_unref(h264Pack);
            continue;
        }

        if (first) {
            if (!h264Pack->flags) {
                av_packet_unref(h264Pack);
                continue;
            } else {
                first = false;
            }
        }

        // std::cout << "come in send data, size is : " << h264Pack->size << std::endl;
        stAVPacketInfo pkt;
        pkt.data.data = h264Pack->data;
        pkt.data.len = h264Pack->size;
        std::shared_ptr<Wrapper> wrapper = std::make_shared<Wrapper>();
        std::string data;
        wrapper->WrapAVPacket((stAVPacketInfo*)&pkt, data);

        {
            std::lock_guard<std::mutex> lk(mutex_);
            for (auto ws_info : ws_infos_) {
                websocketpp::lib::error_code ec;
                if (ws_info.second.ws_server_type == 1) {
                    websocketpp::server<websocketpp::config::asio_tls>* ws_server = (websocketpp::server<websocketpp::config::asio_tls>*)ws_info.second.ws_server;
                    ws_server->send(websocketpp::connection_hdl(ws_info.second.wshdl), data, websocketpp::frame::opcode::binary, ec);
                }
                else {
                    websocketpp::server<websocketpp::config::asio>* ws_server = (websocketpp::server<websocketpp::config::asio>*)ws_info.second.ws_server;
                    ws_server->send(websocketpp::connection_hdl(ws_info.second.wshdl), data, websocketpp::frame::opcode::binary, ec);
                }
                if (ec) {
                    std::cout << "send AVPacket failed: " << ec.message() << std::endl;
                }
            }
        }

        av_packet_unref(h264Pack);
    }

#if 0
    // Clean up
    avcodec_free_context(&codec_ctx);
    av_frame_free(&frame);
    XCloseDisplay(display);
#endif

    std::cout << "come out PullStream" << std::endl;
}

void FFmpegPuller::SetHandlers(void* ws_server, websocketpp::connection_hdl hdl, int ws_server_type) {
    std::lock_guard<std::mutex> lk(mutex_);
    if (hdl.expired()) {
        std::cout << "SetHandlers hdl is expired" << std::endl;
        return;
    }
    auto conn_ptr = hdl.lock().get();
    if (ws_infos_.count(conn_ptr)) {
        std::cout << "conn_ptr is already exist" << std::endl;
        return;
    }
    WsHdlInfo info;
    info.ws_server = ws_server;
    info.ws_server_type = ws_server_type;
    info.wshdl = hdl.lock();

    ws_infos_.insert(std::make_pair(conn_ptr, info));
}

void FFmpegPuller::SendCodecInfo(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lk(mutex_);
    if (hdl.expired()) {
        std::cout << "hdl is expired" << std::endl;
        return;
    }
    auto conn_ptr = hdl.lock().get();
    if (!ws_infos_.count(conn_ptr)) {
        std::cout << "ws_infos_ is dont find conn" << std::endl;
        return;
    }
    auto ws_hd_info = ws_infos_[conn_ptr];
    std::shared_ptr<Wrapper> wrapper = std::make_shared<Wrapper>();
    // audio
    stCodecParam *codec_params_A = nullptr;

    // video
    stCodecParam codec_params;
    codec_params.codecType = 0;
    codec_params.codecId = video_codec_id_;
    codec_params.width = width_;
    codec_params.height = height_;
    codec_params.extraData = extradata_;
    codec_params.extraDataSize = extradata_size_;

    std::string wraper_str;
    wrapper->WrapCodecInfo(&codec_params, codec_params_A, wraper_str);
    websocketpp::lib::error_code ec;
    if (ws_hd_info.ws_server_type == 1) {
        websocketpp::server<websocketpp::config::asio_tls>* ws_server = (websocketpp::server<websocketpp::config::asio_tls>*)ws_hd_info.ws_server;
        ws_server->send(websocketpp::connection_hdl(ws_hd_info.wshdl), wraper_str, websocketpp::frame::opcode::binary, ec);
    }
    else {
        websocketpp::server<websocketpp::config::asio>* ws_server = (websocketpp::server<websocketpp::config::asio>*)ws_hd_info.ws_server;
        ws_server->send(websocketpp::connection_hdl(ws_hd_info.wshdl), wraper_str, websocketpp::frame::opcode::binary, ec);
    }
    if (ec) {
        std::cout << "send codec info failed: " << ec.message() << std::endl;
    }
    wasm_start_ = true;
    std::cout << "send codec info success, wasm_start_ is : " << wasm_start_ << std::endl;
}