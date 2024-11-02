#pragma once

#include "inc-ffmpeg.h"
#include "def_stru.h"
#include <string>
#include "DPacketCache.h"
#include "DFrameCache.h"
#include <thread>
#include <functional>

using namespace std;

struct stBmpInfo
{
    uint8_t* data;
    uint32_t len;
};

struct stDecoderInfo
{
    stCodecParam param;
    string extraData;
    bool valid;
};

class DMediaDecoder
{
public:
    DMediaDecoder();
    virtual ~DMediaDecoder();

public:
    void Init();
    void UnInit();
    // int InitDecoder(stCodecParam* paramV, void* extraDataV, stCodecParam* paramA, void* extraDataA);
    int AsyncInitDecoder(stCodecParam* paramV, void* extraDataV, uint32_t extraDataVLen,
        stCodecParam* paramA, void* extraDataA, uint32_t extraDataALen);
    void UnInitDecoder();

    int AsyncDecodePacket(AVPacket* pkt);
    AVFrame* GetFrame() { return m_frmV; }
    AVFrame* GetFrameFromCache();   //must call FreeFrameFromCache to free memory.
    void FreeFrameFromCache(AVFrame* frm);
    AVFrame* GetFrame_BGR24() { return m_frmV_BGR24; }
    stBmpInfo* GetFrame_bmp() { return &m_bmpInfo; }

protected:
    int initDecVByDecInfo();
    int initDecAByDecInfo();

    void thread_work();

protected:
    stDecoderInfo m_decInfoV;   //async init only.
    stDecoderInfo m_decInfoA;   //async init only.

    AVCodecContext* m_decCtxV;
    AVCodecContext* m_decCtxA;
    AVFrame* m_frmV;
    AVFrame* m_frmV_BGR24;
    stBmpInfo m_bmpInfo;
    int m_idxV;

    DPacketCache m_pktCacheV;
    DFrameCache m_frmCacheV;

private:
    mutex m_mtx_bufcache;
    uint32_t m_max_cachesize;

    bool m_working;
    thread m_thWork;

    uint32_t m_count = 0;
};