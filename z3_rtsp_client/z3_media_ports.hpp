#ifndef _Z3_MEDIA_PORTS_HPP_
#define _Z3_MEDIA_PORTS_HPP_

#include "z3_async_obj.hpp"

namespace Z3 {

        class Bitmap : public MemoryObject
        {
        public:
                Bitmap();
                virtual ~Bitmap();

                int     Alloc(unsigned int nSize);
                void    Free();

                void    SetZero();
                int     TestAndSet(unsigned int nBit);
                void    Reset(unsigned int nBit);
                void    SetBits(unsigned int nBit);

        private:
                uint32_t        m_nBits;
                char            *m_pMap;
        };

        class PortsCtl : public MemoryObject
        {
        public:
                PortsCtl();
                virtual ~PortsCtl();

        protected:
                int     SetRange(unsigned short nLow, unsigned short nHigh);
                void    SetReserved(unsigned short nPort);
                
                int     LockAndGetOne(unsigned short *pnPort);
                void    LockAndPutOne(unsigned short nPort);
                int     LockAndGetTwo(unsigned short *pnLow, unsigned short *pnHigh);
                void    LockAndPutTwo(unsigned short nLow, unsigned short nHigh);

                int     GetOne(unsigned short *pnPort);
                void    PutOne(unsigned short nPort);

                int     GetTwo(unsigned short *pnLow, unsigned short *pnHigh);
                void    PutTwo(unsigned short nLow, unsigned short nHigh);

        private:
                Lock            m_Lock;
                uint16_t        m_nMinPort;
                uint16_t        m_nMaxPort;
                uint16_t        m_nUsed;
                uint16_t        m_nNext;
                uint16_t        m_nReserved;

                Bitmap          *m_pBitmap;
        };

        class MediaPorts : public PortsCtl
        {
        public:
                static MediaPorts*     GetInstance();
                static void            Destroy();

                int     SetDefaultrRange();
                int     SetRange(unsigned short nMin, unsigned short nMax);
                
                // 设置要排除的端口
                void    SetResevedPort(unsigned short nPort);

                // 获得一个端口
                unsigned short  GetOnePort();

                // 释放一个端口
                void            PutOnePort(unsigned short nPort);

                // 获得一对端口
                int     GetPairPorts(unsigned short *pnLow, unsigned short *pnHigh);

                // 释放一对端口
                void    PutPairPorts(unsigned short nLow, unsigned short nHigh);

                // 获得端口范围
                int     GetPortsRange(unsigned short *pnLow, unsigned short *pnHigh);

        protected:
                MediaPorts();
                virtual ~MediaPorts();

        private:
                static MediaPorts*     m_pInstance;
        };

};

#endif