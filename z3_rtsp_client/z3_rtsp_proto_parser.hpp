#ifndef _Z3_RTSP_PROTO_PARSER_HPP_
#define _Z3_RTSP_PROTO_PARSER_HPP_

#include "z3_proto_parser.hpp"
#include "z3_rtsp_msg.hpp"

namespace Z3 {

        class RtspProtoParser : public ProtoParser
        {
        public:
                RtspProtoParser();

                virtual int Parse(const char *pBuf, unsigned int nLen, void *pData, Msg **pMsg /* output */);

        protected:
                virtual ~RtspProtoParser();

                int     ParseRequestLine(char *pBuf, RtspMsg *pMsg);
                int     ParseResponseStatus(char *pBuf, RtspMsg *pMsg);
                
                /* parsing lines means reading a Key: Value pair */
                int     ParseLine(char *pBuf, RtspMsg *pMsg);

                int     ParseProtocolVersion(char *pszProtocol, RTSP_VERSION *pVersion);

        private:
                RtspMsg         *m_pRtspMsg;
                bool		m_bMsgCompleted;

                BUILDER_STATE   m_state;
                unsigned int    m_nLine;

                char            *m_pBuf;
                unsigned int    m_nBufSize;
                unsigned int    m_nBufBytes;    // Available bytes in m_pBuf
                
                
                /*
                 * 标记每次进入函数Parse时应该从“哪里”开始处理
                 */
                unsigned int    m_nOffset;

        };
};

#endif