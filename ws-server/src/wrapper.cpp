#include "wrapper.h"

Wrapper::Wrapper()
{

}

Wrapper::~Wrapper()
{

}

int Wrapper::WrapCodecInfo(stCodecParam* paramV, stCodecParam* paramA, std::string& ostr)
{
    int paramNum = 0;
    int extraSize = 0;
    uint32_t extraSizeV = 0;
    uint32_t extraSizeA = 0;
    if (paramV) {
        ++paramNum;
        extraSizeV = paramV->extraDataSize;
        extraSize += extraSizeV;
        HtonCodecParam(paramV);
    }
    if (paramA) {
        ++paramNum;
        extraSizeA = paramA->extraDataSize;
        extraSize += extraSizeA;
        HtonCodecParam(paramA);
    }
    if (paramNum == 0) {
        return -1;
    }

    uint32_t frmHdrSize = sizeof(stIntervalFrameHdr);
    uint32_t paramSize = sizeof(stCodecParam) - sizeof(void*);
    uint32_t len = frmHdrSize + paramSize * paramNum + extraSize;
    uint8_t* buf = new uint8_t[len];
    uint8_t* pTmp = buf;

    buf[0] = 0x24; //sc
    buf[1] = 1; //type:codec params
    pTmp += 2;

    if (paramV) {
        memcpy(pTmp, paramV, paramSize);
        pTmp += paramSize;
        if (extraSizeV > 0 && paramV->extraData) {
            memcpy(pTmp, paramV->extraData, extraSizeV);
            pTmp += extraSizeV;
        }
    }
    if (paramA) {
        memcpy(pTmp, paramA, paramSize);
        pTmp += paramSize;
        memcpy(pTmp, paramA->extraData, extraSizeA);
        pTmp += extraSizeA;
    }

    ostr.assign((char*)buf, len);

    delete[] buf;
    buf = NULL;

    return 0;
}

int Wrapper::WrapAVPacket(stAVPacketInfo* pkt, std::string& opkt)
{
    if (!pkt) {
        return -1;
    }

    pkt->hdr.pts = htonl((uint32_t)pkt->hdr.pts);
    pkt->hdr.dts = htonl((uint32_t)pkt->hdr.dts);
    pkt->hdr.timebaseNum = htonl(pkt->hdr.timebaseNum);
    pkt->hdr.timebaseDen = htonl(pkt->hdr.timebaseDen);
    pkt->hdr.flag = htonl(pkt->hdr.flag);
    pkt->hdr.codecId = htonl(pkt->hdr.codecId);

    uint32_t frmHdrSize = sizeof(stIntervalFrameHdr);
    uint32_t pktHdrSize = sizeof(stAVHeaderInfo);
    uint32_t len = frmHdrSize + pktHdrSize + 4 + pkt->data.len;
    uint8_t* buf = new uint8_t[len];
    uint32_t pktDataLen_n = htonl(pkt->data.len);
    uint8_t* pTmp = buf;


    pTmp[0] = 0x24; //sc
    pTmp[1] = 0;  //type:packet
    pTmp += 2;
    memcpy(pTmp, &pkt->hdr, pktHdrSize);
    pTmp += pktHdrSize;
    memcpy(pTmp, &pktDataLen_n, 4);
    pTmp += 4;
    memcpy(pTmp, pkt->data.data, pkt->data.len);
    // LOG(INFO) << "len is : " << pkt->data.len;
    opkt.assign((char*)buf, len);

    delete[] buf;
    buf = NULL;

    return 0;
}

void Wrapper::HtonCodecParam(stCodecParam* param)
{
    assert(param != NULL);
    param->codecType = htonl(param->codecType);
    param->codecId = htonl(param->codecId);
    param->width = htonl(param->width);
    param->height = htonl(param->height);
    param->pktTimebaseNum = htonl(param->pktTimebaseNum);
    param->pktTimebaseDen = htonl(param->pktTimebaseDen);
    // param->sampleRate = htonl(param->sampleRate);
    // param->sampleBits = htonl(param->sampleBits);
    // param->channel = htonl(param->channel);
    if (param->extraData) {
        param->extraDataSize = htonl(param->extraDataSize);
    } else {
        param->extraDataSize = 0;
    }
}