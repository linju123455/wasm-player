/*================================================================
*   Copyright (c) 2024, Inc. All Rights Reserved.
*   
*   @file：wrapper.h
*   @author：linju
*   @email：15013144713@163.com
*   @date ：2024-05-20
*   @berief： Wrapper netdata private header
*
================================================================*/
#pragma once
#include <iostream>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>

struct stCodecParam
{
	uint32_t codecType;	//0:v, 1:a
	uint32_t codecId;
	uint32_t width;
	uint32_t height;
	uint32_t gopSize;
	uint32_t pktTimebaseNum;
	uint32_t pktTimebaseDen;
	// uint32_t sampleRate;
	// uint32_t sampleBits;
	// uint32_t channel;
	uint32_t extraDataSize;
	uint8_t *extraData; // wasm32位，指针类型大小为4
};

struct stAVHeaderInfo
{
    uint32_t pts;
    uint32_t dts;
    uint32_t timebaseNum;
    uint32_t timebaseDen;
    uint32_t flag;  //is key frame
    uint32_t codecId;

    //audio attribute
    uint32_t channel;
    uint32_t sampleRate;
    uint32_t sampleBits;
};

struct stAVDataInfo
{
    uint32_t len;
    uint8_t* data;
};

struct stAVPacketInfo
{
    stAVHeaderInfo hdr;
    stAVDataInfo data;
};

struct stIntervalFrameHdr
{
    uint8_t sc; //0x24
    uint8_t frmType; //0: av packet info, 1: codec params
};

class Wrapper {
public:
    Wrapper();
    virtual ~Wrapper();

    int WrapCodecInfo(stCodecParam* paramV, stCodecParam* paramA, std::string& ostr);

    int WrapAVPacket(stAVPacketInfo* pkt, std::string& opkt);

private:
    void HtonCodecParam(stCodecParam* param);
};

