#ifndef _Z3_RTCP_MSG_HPP_
#define _Z3_RTCP_MSG_HPP_

#include "z3_rtcp.hpp"
#include "z3_msg.hpp"

namespace Z3 {

        class RTCPPacket : public MemoryObject
        {
        public:
                RTCPPacket();
                virtual ~RTCPPacket();
                
                void            InitPacket(char *pBuf, size_t nBufSize);
                unsigned char   GetPayloadType();
                
                uint16_t        GetPacketLength();

        protected:
                char            *m_pPacket;
                uint32_t        m_nPacketSize;
        };

        class RTCPPacketList : public Msg
        {
        public:
                RTCPPacketList();

                virtual unsigned int    ProtoID();

                int             Push(RTCPPacket *pPacket);
                RTCPPacket*     Pop(void);

                inline bool     IsEmpty() { return m_lstRtcpPackets.empty(); }

        protected:
                virtual ~RTCPPacketList();

                std::list<RTCPPacket *> m_lstRtcpPackets;
        };

        // Report Block
        class RB : public MemoryObject
        {
        public:
                RB();
                virtual ~RB();

                void            InitBlock(char *pBuf, size_t nBufSize);

                uint32_t        GetSSRC();
                uint32_t        GetLostRate();
                uint32_t        GetLost();      //  lost count
                uint32_t        GetLastSeq();
                uint32_t        GetJitter();
                uint32_t        GetLSR();
                uint32_t        GetDLSR();

        protected:
                char            *m_pBlock;
                uint32_t        m_nBlockSize;

        };

        class SR : public RTCPPacket
        {
        public:
                SR();
                virtual ~SR();

                void            InitPacket(char *pBuf, size_t nBufSize);

                uint32_t        GetRC();

                uint32_t        GetSSRC();

                uint64_t        GetNTPTimestamp();

                unsigned long   GetRTPTimestamp();

                uint32_t        GetSenderPktCount();
                uint32_t        GetSenderBytes();

                uint32_t        GetRBCount();
                RB *            GetRB(uint32_t nIndex);

        protected:
                std::vector<RB> m_vectorRBs;
        };

        class RR : public RTCPPacket
        {
        public:
                RR();
                virtual ~RR();

                void            InitPacket(char *pBuf, size_t nBufSize);

                uint32_t        GetRC();
                uint32_t        GetSSRC();

                uint32_t        GetRBCount();
                RB *            GetRB(uint32_t nIndex);

        protected:
                std::vector<RB> m_vectorRBs;
        };

        class SrcDesc
        {
        public:
                SrcDesc();
                virtual ~SrcDesc();

                void    InitChunk(char *pChunk, size_t nChunkSize);

                uint32_t        GetSSRC();
                uint8_t         GetType();
                uint8_t         GetLength();
                uint8_t         GetItemName(char *pBuf, size_t nBufSize);

        protected:
                char            *m_pChunk;
                uint32_t        m_nChunkSize;
        };

        class SDES : public RTCPPacket
        {
        public:
                SDES();
                virtual ~SDES();

                void            InitPacket(char *pBuf, size_t nBufSize);

                uint32_t        GetSC();

        protected:
                std::vector<SrcDesc>     m_vectorDESCs;
        };
};

#endif