#include "z3_common.hpp"
#include "z3_media_ports.hpp"
#include "z3_rtsp_error.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

#define ALIGN(size, align)	((size + align - 1)&(~(align - 1)))
#define BIT_BYTES(size)		(ALIGN((size), sizeof(char))/sizeof(char));

#define DEFAULT_RANGE_LOWER		20000
#define DEFAULT_RANGE_UPPER		30000

Bitmap::Bitmap()
        : m_nBits(0)
        , m_pMap(NULL)
{
}

Bitmap::~Bitmap()
{
        Z3_FREE_POINTER(m_pMap);
}

int Bitmap::Alloc(unsigned int nSize)
{
        unsigned int nBytes;

        assert(m_pMap == NULL);

        nBytes = BIT_BYTES(nSize);
        m_pMap = (char *)z3_malloc(nBytes);
        if (m_pMap)
        {
                m_nBits = nSize;
                return 0;
        }

        return ENOMEM;
}

void Bitmap::Free()
{
        Z3_FREE_POINTER(m_pMap);
        m_nBits = 0;
}

void Bitmap::SetZero()
{
        unsigned int nBytes;
        if (m_pMap)
        {
                nBytes = BIT_BYTES(m_nBits);
                memset(m_pMap, 0, nBytes);
        }
}

int Bitmap::TestAndSet(unsigned int nBit)
{
        unsigned int nOld, nOffset;

        assert(nBit <= m_nBits);

        nOld = m_pMap[nBit / sizeof(char)];
        nOffset = (nBit & (sizeof(char) - 1));

        m_pMap[nBit/sizeof(char)] |= 1 << nOffset;

        return nOld & ( 1 << nOffset);
}

void Bitmap::Reset(unsigned int nBit)
{
        unsigned int nOld, nOffset;

        assert(nBit <= m_nBits);

        nOld = m_pMap[nBit / sizeof(char)];
        nOffset = (nBit & (sizeof(char) - 1));

        if (!(nOld & (1 << nOffset)))
        {
                TRACE_ERROR("Bit \'%d\' was reset twice in bitmap 0x%p", nBit, this);
                assert(false);
        }

        m_pMap[nBit/sizeof(char)] &= ~(1 << nOffset);
}

void Bitmap::SetBits(unsigned int nBit)
{
        assert(nBit < m_nBits);
        m_nBits = nBit;
}

PortsCtl::PortsCtl()
        : m_nMinPort(0)
        , m_nMaxPort(0)
        , m_nUsed(0)
        , m_nNext(1)
        , m_pBitmap(NULL)
{        
}

PortsCtl::~PortsCtl()
{
        Z3_DELETE_OBJ(m_pBitmap);
}

int PortsCtl::SetRange(uint16_t nLow, uint16_t nHigh)
{        
        uint16_t  nSize;

        assert(nLow > 0 && nHigh > 0);
        assert(nHigh > nLow);

        nSize = nHigh - nLow;

        m_Lock.On();

        if (nSize > (m_nMaxPort - m_nMinPort))
        {
                Z3_DELETE_OBJ(m_pBitmap);
                m_pBitmap = new Bitmap;
                if (m_pBitmap == NULL)
                {
                        m_Lock.Off();
                        return ENOMEM;
                }

                m_pBitmap->Alloc(nSize);
        }
        else
        {
                m_pBitmap->SetBits(nSize);
        }

        m_pBitmap->SetZero();
        
        m_nMinPort      = nLow;
        m_nMaxPort      = nHigh;
        m_nUsed         = 0;
        m_nNext         = m_nMinPort;
        m_nReserved     = 0;

        m_Lock.Off();

        return 0;
}

void PortsCtl::SetReserved(uint16_t nPort)
{        
        m_Lock.On();

        if (nPort > m_nMaxPort || nPort < m_nMinPort)
        {
                m_Lock.Off();
                return;
        }

        if (!m_pBitmap->TestAndSet(nPort - m_nMinPort))
               m_nReserved++;

        m_Lock.Off();
}

int PortsCtl::GetOne(uint16_t *pnPort)
{
        uint16_t nTotal, nPort;

        nTotal = m_nMaxPort - m_nMinPort + 1;
        if (m_nUsed + m_nReserved >= nTotal)
        {
                return Z3_RTSP_EOUTOFPORTS;
        }

        while(true)
        {
                if (--nTotal < 0)
                        break;

                if (!m_pBitmap->TestAndSet(m_nNext - m_nMinPort))
                {
                        nPort = m_nNext;

                        m_nUsed++;
                        m_nNext++;

                        if (m_nNext > m_nMaxPort)
                                m_nNext = m_nMinPort;

                        *pnPort = nPort;

                        return 0;
                }

                if (++m_nNext > m_nMaxPort)
                        m_nNext = m_nMinPort;
        }

        return Z3_RTSP_EOUTOFPORTS;
}

int PortsCtl::LockAndGetOne(uint16_t *pnPort)
{
        int     nError;

        m_Lock.On();
        nError = GetOne(pnPort);
        m_Lock.Off();

        return nError;
}

void PortsCtl::PutOne(uint16_t nPort)
{
 
        if (nPort > m_nMaxPort || nPort < m_nMinPort)
        {
                TRACE_ERROR("Failed to put port %d, not in range[%d, %d]\r\n", 
                        nPort, m_nMinPort, m_nMaxPort);
                
                m_Lock.Off();
                return;
        }

        m_pBitmap->Reset(nPort - m_nMinPort);
        --m_nUsed;

}

void PortsCtl::LockAndPutOne(uint16_t nPort)
{
        m_Lock.On();
        PutOne(nPort);
        m_Lock.Off();
}

int PortsCtl::GetTwo(uint16_t *pnLow, uint16_t *pnHigh)
{
        int nError;
        uint16_t nTotal;

        nTotal = m_nMaxPort - m_nMinPort;
        if ((m_nUsed + m_nReserved + 2) > nTotal)
        {
                return Z3_RTSP_EOUTOFPORTS;
        }

        while (true)
        {
                for (;;)
                {
                        nError = GetOne(pnLow);
                        if (nError != 0)
                                return nError;

                        if (*pnLow & 0x01)
                        {
                                PutOne(*pnLow);
                                continue;
                        }

                        break;
                }

                for (;;)
                {
                        nError = GetOne(pnHigh);
                        if (nError != 0)
                        {
                                PutOne(*pnLow);
                                return nError;
                        }

                        if (*pnHigh == *pnLow + 1)      // got it
                                return 0;

                        PutOne(*pnLow);

                        if (*pnHigh & 0x01)
                        {
                                PutOne(*pnHigh);
                                break;
                        }
                        else
                        {
                                *pnLow = *pnHigh;
                        }
                }
        }

        return nError;
}

void PortsCtl::PutTwo(uint16_t nLow, uint16_t nHigh)
{
        PutOne(nLow);
        PutOne(nHigh);
}

void PortsCtl::LockAndPutTwo(uint16_t nLow, uint16_t nHigh)
{
        m_Lock.On();
        PutTwo(nLow, nHigh);
        m_Lock.Off();
}

int PortsCtl::LockAndGetTwo(uint16_t *pnLow, uint16_t *pnHigh)
{
        int nError;

        m_Lock.On();
        nError = GetTwo(pnLow, pnHigh);
        m_Lock.Off();

        return nError;
}

MediaPorts::MediaPorts()
{
}

MediaPorts::~MediaPorts()
{
}

MediaPorts* MediaPorts::m_pInstance = NULL;
MediaPorts* MediaPorts::GetInstance()
{
        if (m_pInstance == NULL)
        {
                m_pInstance = new MediaPorts;
                m_pInstance->SetDefaultrRange();
        }

        return m_pInstance;
}

void MediaPorts::Destroy()
{
        if (m_pInstance)
        {
                delete m_pInstance;
                m_pInstance = NULL;
        }
}

int MediaPorts::SetDefaultrRange()
{
        return SetRange(DEFAULT_RANGE_LOWER, DEFAULT_RANGE_UPPER);
}

int MediaPorts::SetRange(uint16_t nMin, uint16_t nMax)
{
        return PortsCtl::SetRange(nMin, nMax);
}

void MediaPorts::SetResevedPort(uint16_t nPort)
{
        SetReserved(nPort);
}

uint16_t MediaPorts::GetOnePort()
{
        int nError;
        uint16_t nPort;

        nError = LockAndGetOne(&nPort);
        if (nError == 0)
                return nPort;
        else
                return 0;
}

void MediaPorts::PutOnePort(uint16_t nPort)
{
        LockAndPutOne(nPort);
}

int MediaPorts::GetPairPorts(uint16_t *pnLow, uint16_t *pnHigh)
{
        return LockAndGetTwo(pnLow, pnHigh);
}

void MediaPorts::PutPairPorts(uint16_t nLow, uint16_t nHigh)
{
        return LockAndPutTwo(nLow, nHigh);
}