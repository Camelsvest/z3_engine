#include "z3_common.hpp"
#include "z3_h264_frame.hpp"

using namespace Z3;

H264Frame::H264Frame()
        : m_pFrameBuf(NULL)
        , m_lCurrPos(0)
        , m_lMaxFrameSize(0)
        , m_lTimestamp(0)
{
}

H264Frame::~H264Frame()
{
        Z3_FREE_POINTER(m_pFrameBuf);
}

bool H264Frame::Allocate(unsigned long lMaxSize)
{
        if (m_lMaxFrameSize < lMaxSize)
        {
                Z3_FREE_POINTER(m_pFrameBuf);
                m_pFrameBuf = (char *)z3_malloc(lMaxSize);
                if (m_pFrameBuf)
                {
                        m_lMaxFrameSize = lMaxSize;
                        return true;
                }                
        }

        return false;
}

unsigned int H264Frame::GetFrameType()
{
        return H264_VIDEO_FRAME;
}

unsigned long H264Frame::GetFrameSize()
{
        return m_lCurrPos;
}

const char* H264Frame::GetFrameBuffer()
{
        return m_pFrameBuf;
}

bool H264Frame::AddNalStartCode()
{
        char const start_code[4] = {0x00, 0x00, 0x00, 0x01};

        if (m_lCurrPos + 4 > m_lMaxFrameSize)
                return false;

        memcpy(m_pFrameBuf + m_lCurrPos, start_code, sizeof(start_code));
        m_lCurrPos += 4;

        return true;
}

unsigned long H264Frame::FillData(const char *pData, size_t nDataSize)
{
        if (m_lCurrPos  + nDataSize > m_lMaxFrameSize)
        {
                assert(false);
                return 0;
        }

        memcpy(m_pFrameBuf + m_lCurrPos, pData, nDataSize);
        m_lCurrPos += nDataSize;

        return nDataSize;
}

void H264Frame::Reset()
{
        m_lCurrPos = 0;
        m_lTimestamp = 0;
}