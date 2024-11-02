#include "DMediaDecoder.h"
#include <assert.h>
#include <iostream>
#pragma pack(push, 1)
// bitmap file header
typedef struct BitmapFileHeader
{
    char identifier[2];  // 文件类型,必须为BM
    uint32_t fileSize;   // 整个BMP文件大小,单位字节
    uint16_t reserved1;  // 保留，必须为0
    uint16_t reserved2;  // 保留，必须为0
    uint32_t dataOffset; // 文件头到实际图像数据之间的偏移
} BITMAP_FILE_HEADER;

// bitmap info header
typedef struct BitmapInfoHeader
{
    uint32_t infoSize;        // BitmapInfoHeader结构体大小，单位字节
    uint32_t width;           // 图形宽度，单位像素
    uint32_t height;          // 图形高度，单位像素
    int16_t planes;           // 目标设备级别，必须为1
    int16_t bitsPerPixel;     // 颜色深度，每个像素所需的位数
    uint32_t compression;     // 位图的压缩类型
    uint32_t dataSize;        // 位图的大小，单位字节
    uint32_t hresolution;     // 位图水平分辨率，每米像素数
    uint32_t vresolution;     // 位图垂直分辨率，每米像素数
    uint32_t useColors;       // 位图实际使用的颜色表中的颜色数
    uint32_t importantColors; // 位图显示过程中重要的颜色数
} BITMAP_INFO_HEADER;
#pragma pack(pop)

DMediaDecoder::DMediaDecoder()
    : m_decCtxV(NULL),
    m_decCtxA(NULL),
    m_max_cachesize(100),
    m_frmV(NULL),
    m_frmV_BGR24(NULL)
{
    av_log_set_level(AV_LOG_ERROR);
    memset(&m_bmpInfo, 0, sizeof(stBmpInfo));

    m_decInfoV.valid = false;
    m_decInfoA.valid = false;
}

DMediaDecoder::~DMediaDecoder()
{
    if (m_frmV)
    {
        av_frame_free(&m_frmV);
    }
    if (m_frmV_BGR24)
    {
        av_frame_free(&m_frmV_BGR24);
    }

    UnInitDecoder();
}

void DMediaDecoder::Init()
{
    m_working = true;
    m_thWork = thread(bind(&DMediaDecoder::thread_work, this));
    printf("m_pktCacheV is : %p\n", &m_pktCacheV);
}

void DMediaDecoder::UnInit()
{
    m_working = false;
    if (m_thWork.joinable())
    {
        m_thWork.join();
    }
    printf("thread exit...\n");
}

int DMediaDecoder::AsyncInitDecoder(stCodecParam* paramV, void* extraDataV, uint32_t extraDataVLen,
    stCodecParam* paramA, void* extraDataA, uint32_t extraDataALen)
{
    // ignore audio
    if (!paramV)
    {
        printf("invalid codec param[v]!\n");
        return -1;
    }

    m_decInfoV.param = *paramV;
    m_decInfoV.extraData.assign((char*)extraDataV, extraDataVLen);
    m_decInfoV.valid = true;

    return 0;
}

void DMediaDecoder::UnInitDecoder()
{
    if (m_decCtxV)
    {
        avcodec_free_context(&m_decCtxV);
    }
    if (m_decCtxA)
    {
        avcodec_free_context(&m_decCtxV);
    }
}

int DMediaDecoder::AsyncDecodePacket(AVPacket* pkt)
{
    if (!pkt)
    {
        printf("invalid args\n");
        return -1;
    }

    if (!m_decCtxV)
    {
        printf("video decoder not initialized\n");
        return -1;
    }
    
    // std::cout << "pak size is : " << m_pktCacheV.Size() << std::endl;
    if (m_pktCacheV.AddPacket(pkt) < 0)
    {
        // printf("packet cache full, clear..., m_pktCacheV is : %p\n", &m_pktCacheV);
        m_pktCacheV.ClearUntilKey();
        m_pktCacheV.AddPacket(pkt);
    }

    return 0;
}

AVFrame* DMediaDecoder::GetFrameFromCache()
{
    return m_frmCacheV.GetFrame();
}

void DMediaDecoder::FreeFrameFromCache(AVFrame* frm)
{
    if (frm)
    {
        av_frame_free(&frm);
    }
}

int DMediaDecoder::initDecVByDecInfo()
{
    if (!m_decInfoV.valid) {
        // printf("dec info v not valid yet!\n");
        return -1;
    }

    UnInitDecoder();

    AVCodecParameters* codecParam = (AVCodecParameters*)av_malloc(sizeof(AVCodecParameters));
    
    AVCodec* decV = avcodec_find_decoder((AVCodecID)m_decInfoV.param.codecId);
    if (!decV) {
        printf("AVCodec not find!\n");
        return -1;
    }
    m_decCtxV = avcodec_alloc_context3(decV);

    m_decCtxV->pkt_timebase = { (int)m_decInfoV.param.pktTimebaseNum, (int)m_decInfoV.param.pktTimebaseDen };

    codecParam->codec_type = AVMEDIA_TYPE_VIDEO;
    codecParam->codec_id = (AVCodecID)m_decInfoV.param.codecId;
    printf("codec id is : %d\n", codecParam->codec_id);
    codecParam->width = m_decInfoV.param.width;
    printf("width is : %d\n", codecParam->width);
    codecParam->height = m_decInfoV.param.height;
    printf("height is : %d\n", codecParam->height);
    memset(codecParam, 0, sizeof(AVCodecParameters));
    codecParam->extradata_size = m_decInfoV.param.extraDataSize;
    printf("extraDataSize is : %d\n", codecParam->extradata_size);
    codecParam->extradata = (uint8_t*)av_malloc(codecParam->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
    memset(codecParam->extradata, 0, codecParam->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
    if (m_decInfoV.extraData.length() > 0)
    {
        memcpy(codecParam->extradata, m_decInfoV.extraData.c_str(), m_decInfoV.extraData.length());
        // for (int i = 0; i < codecParam->extradata_size; i++)
        // {
        //     printf("%02X ", codecParam->extradata[i]);
        //     if((i+1)%16==0)
        //     {
        //         printf("\n");
        //     }
        // }
        // printf("\n");
    }
    int ret = avcodec_parameters_to_context(m_decCtxV, codecParam);
    if (ret < 0)
    {
        // printf("set video codec context failed: %s\n", translate_ffmpeg_error(ret).c_str());
        printf("set video codec context failed: %d\n", -ret);
        UnInitDecoder();
        return -1;
    }
    avcodec_parameters_free(&codecParam);

    // m_decCtxV->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    ret = avcodec_open2(m_decCtxV, decV, NULL);
    if (ret < 0)
    {
        // printf("open video decoder failed: %s\n", translate_ffmpeg_error(ret).c_str());
        printf("open video decoder failed: %d\n", -ret);
        UnInitDecoder();
        return -1;
    }

    m_pktCacheV.ClearAll();

    printf("video decoder initialized.\n");

    return 0;
}

int DMediaDecoder::initDecAByDecInfo()
{
    printf("audio not supported\n");

    return -1;
}

void DMediaDecoder::thread_work()
{
    //ignore audio

    AVFrame* frmVCached = NULL;
    while (m_working)
    {
        if (m_decCtxV == NULL) {
            if (initDecVByDecInfo() < 0) {
                // printf("initDecVByDecInfo error...\n");
                this_thread::sleep_for(chrono::milliseconds(20));
                continue;
            }
        }

        AVFrame* frmV = av_frame_alloc();
        AVPacket* pktCached = m_pktCacheV.GetPacket();
        if (!pktCached)
        {
            av_frame_free(&frmV);
            this_thread::sleep_for(chrono::milliseconds(15));
            continue;
        }

        int got = 0;
        if (m_decCtxV == nullptr) {
            printf("m_decCtxV is null\n");
        }

        int ret = avcodec_decode_video2(m_decCtxV, frmV, &got, pktCached);
        av_packet_free(&pktCached);
        if (ret < 0) {
            char buf[64] = {0};
            av_strerror(ret, buf, sizeof(buf));
            printf("%s\n", buf);
        }
        if (got <= 0)
        {
            printf("decode error...\n");
            av_frame_free(&frmV);
            continue;
        }

        if (m_frmCacheV.AddFrame(frmV) < 0)
        {
            // printf("AddFrame error...\n");
            m_frmCacheV.RemoveFront();
            m_frmCacheV.AddFrame(frmV);
        }

        av_frame_free(&frmV);
    }

    printf("exit decode worker...\n");
}