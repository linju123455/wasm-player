#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <map>
#include "DMediaDecoder.h"
#include "DMediaRenderer.h"
#include "utils.h"
#include <fstream>
#include <emscripten.h>

#ifndef EM_PORT_API
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#if defined(__cplusplus)
#define EM_PORT_API(rettype) extern "C" rettype EMSCRIPTEN_KEEPALIVE
#else
#define EM_PORT_API(rettype) rettype EMSCRIPTEN_KEEPALIVE
#endif
#else
#if defined(__cplusplus)
#define EM_PORT_API(rettype) extern "C" rettype
#else
#define EM_PORT_API(rettype) rettype
#endif
#endif
#endif

int async_proc_codec_params(DMediaDecoder *processor, uint8_t *data, uint32_t len);
int async_proc_av_packet(DMediaDecoder *processor, uint8_t *data, uint32_t len);

DMediaDecoder *g_Decoder = NULL;
DMediaRenderer *g_renderer = NULL;
static int g_rendererWidth = 0;
static int g_rendererHeight = 0;

void mainloop()
{
    if (!g_Decoder)
    {
        return;
    }

    AVFrame *frm = g_Decoder->GetFrameFromCache();
    if (!frm)
    {
        return;
    }

    g_renderer->DrawAVFrame(frm);

    g_Decoder->FreeFrameFromCache(frm);
}

EM_PORT_API(void *)
CreateProcessor(int w, int h)
{
    DMediaDecoder* processor = new DMediaDecoder();
    if (!processor)
    {
        printf("create processor failed!\n");
        return NULL;
    }
    processor->Init();
    g_Decoder = processor;

    g_rendererWidth = w;
    g_rendererHeight = h;
    printf("windows w is : %d, h is : %d\n", g_rendererWidth, g_rendererHeight);

    g_renderer = new DMediaRenderer();
    if (!g_renderer)
    {
        printf("create renderer failed!\n");
        if (processor)
        {
            processor->UnInit();
            printf("destroy processor:%p\n", processor);
            delete processor;
            processor = NULL;
            if (g_Decoder != NULL) {
                g_Decoder = NULL;
            }
        }
        return NULL;
    }
    g_renderer->Init(g_rendererWidth, g_rendererHeight);

    emscripten_set_main_loop(mainloop, 0, 0);
    printf("start main loop success!\n");
    return processor;
}

EM_PORT_API(void)
DestroyProcessor(DMediaDecoder* processor)
{
    if (processor)
    {
        processor->UnInit();
        printf("destroy processor:%p\n", processor);
        delete processor;
        processor = NULL;
        if (g_Decoder != NULL) {
            g_Decoder = NULL;
        }
    }
    
    if (g_renderer)
    {
        printf("destroy renderer:%p\n", g_renderer);
        delete g_renderer;
        g_renderer = NULL;
    }

    emscripten_cancel_main_loop();
    printf("cancel main loop success!\n");
}

EM_PORT_API(int)
AsyncParsePacket(DMediaDecoder* processor, uint8_t *data, uint32_t len)
{
    if (!processor || !data || len < 2 || data[0] != 0x24)
    {
        printf("invalid packet data! data:%p, len:%d, sc: 0x%02x\n", data, len, data[0]);
        return -1;
    }

    switch (data[1])
    {
    case 0:
        // printf("get frame packet...\n");
        return async_proc_av_packet(processor, data + 2, len - 2);
    case 1:
        printf("get decode parameters...\n");
        return async_proc_codec_params(processor, data + 2, len - 2);
    default:
        printf("unsupport packet type");
        return -1;
    }

    return 0;
}

int async_proc_codec_params(DMediaDecoder *processor, uint8_t *data, uint32_t len)
{
    if (!processor || !data || len < (sizeof(stCodecParam) - sizeof(uint8_t*)))
    {
        printf("async proc codec params failed: invalid args\n");
        return -1;
    }

    stCodecParam *param = (stCodecParam *)data;
    param->codecType = dntohl(param->codecType);
    param->codecId = dntohl(param->codecId);
    param->width = dntohl(param->width);
    param->height = dntohl(param->height);
    param->gopSize = dntohl(param->gopSize);
    param->pktTimebaseNum = dntohl(param->pktTimebaseNum);
    param->pktTimebaseDen = dntohl(param->pktTimebaseDen);
    // param->sampleRate = dntohl(param->sampleRate);
    // param->sampleBits = dntohl(param->sampleBits);
    // param->channel = dntohl(param->channel);
    param->extraDataSize = dntohl(param->extraDataSize);

    // 如果存在sps、pps，则直接跟在结构体后面
    uint8_t *extraDataV = data + (sizeof(stCodecParam) - sizeof(uint8_t*));
    // ignore audio

    return processor->AsyncInitDecoder(param, extraDataV, param->extraDataSize, NULL, NULL, 0);
}

int async_proc_av_packet(DMediaDecoder *processor, uint8_t *data, uint32_t len)
{
    if (!processor || !data || len == 0)
    {
        printf("proc av packet failed: invalid args\n");
        return -1;
    }

    AVPacket *pkt = av_packet_alloc();
    av_init_packet(pkt);

    stAVPacketInfo *pktInfo = (stAVPacketInfo *)data;
    uint32_t dataSize = dntohl(pktInfo->data.len);
    av_new_packet(pkt, dataSize);

    pkt->pts = dntohl(pktInfo->hdr.pts);
    pkt->dts = dntohl(pktInfo->hdr.dts);
    pkt->size = dataSize;
    pkt->flags = pktInfo->hdr.flag;

    memcpy(pkt->data, data + sizeof(stAVPacketInfo), dataSize);
    if (processor->AsyncDecodePacket(pkt) < 0)
    {
        printf("decode packet failed!\n");
        return -1;
    }

    av_packet_free(&pkt);

    return 0;
}