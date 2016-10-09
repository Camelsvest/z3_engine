#ifndef _Z3_RTSP_MSG_HPP_
#define _Z3_RTSP_MSG_HPP_

#include "z3_msg.hpp"
#include "z3_rtsp_def.hpp"

#define DEF_MAX_KEY_VALUE	(8)
#define TOP_MAX_KEY_VALUE	(DEF_MAX_KEY_VALUE * 2)

#define DEF_MAX_VALUE_LEN	(64)
#define MIN(x, y) ((x) < (y) ? (x) : (y))

namespace Z3 {

        typedef enum _BUILDER_STATE
        {
	        STATE_START = 0,
	        STATE_DATA_HEADER,
	        STATE_DATA_BODY,
	        STATE_READ_LINES,
	        STATE_END,
	        STATE_LAST
        } BUILDER_STATE;

        class RtspMsg;

        class RtspKeyValue : public MemoryObject
        {
        public:
                RtspKeyValue();
                ~RtspKeyValue();

                void    FillKeyValue(RTSP_HEADER_FIELD Field, const char *pValue, unsigned int nValueLen);
                void    UnsetKeyValue();
                
                RtspKeyValue& operator= (RtspKeyValue &Key);

                RTSP_HEADER_FIELD       m_field;
                unsigned int            m_nValueLen;
                char                    *m_pValue;
                char                    m_szInit[DEF_MAX_VALUE_LEN];
        };

        class RtspKeyValueArray : public MemoryObject
        {
        public:
                RtspKeyValueArray();
                ~RtspKeyValueArray();

                void            AddKeyValue(RTSP_HEADER_FIELD Field, const char *pValue, unsigned int nValueLen);
                int             TakeKeyValuesToLarger(unsigned int nLargerSize);
                void            PrintEachKeyValue();

                RtspKeyValueArray& operator= (RtspKeyValueArray &Array);

                inline unsigned int     Size() { return m_nSize; }
                inline unsigned int     KeyValueSum() { return m_nKeyValueSum; }
                inline unsigned int     KeyValueSumDecrease() { return --m_nKeyValueSum; }

                RtspKeyValue*           GetRtspKeyValue(unsigned int nIndex);

        private:
                unsigned int    m_nSize;
                unsigned int    m_nKeyValueSum;
                RtspKeyValue    *m_pRtspKeyValues;
        };

        typedef enum _RTSP_MSG_TYPE
        {
                RTSP_MSG_INVALID,
                RTSP_MSG_REQUEST,
                RTSP_MSG_RESPONSE,
                RTSP_MSG_DATA
        } RTSP_MSG_TYPE;

        class RtspMsgType : public MemoryObject
        {
        public:
                RtspMsgType(RTSP_MSG_TYPE msgType);
                virtual ~RtspMsgType();

                inline RTSP_MSG_TYPE    GetMsgType() { return m_msgType; }

        private:
                RTSP_MSG_TYPE   m_msgType;
        };

        class RtspRequest : public RtspMsgType
        {
        public:
                RtspRequest();
                virtual ~RtspRequest();

                int             Init(RTSP_METHOD method, 
                                             const char *pUri,
                                             unsigned int nUriLength,
                                             RTSP_VERSION ver = RTSP_VERSION_1_0);

                inline RTSP_VERSION     GetVersion() { return m_version; }
                inline void             SetVersion(RTSP_VERSION ver) { m_version = ver; }

                inline RTSP_METHOD      GetMethod() { return m_method; }
                inline void             SetMethod(RTSP_METHOD method) { m_method = method; }

                inline const char*      GetUri() { return m_pURI; }

        protected:

        private:
		RTSP_METHOD	m_method;
		char 	        *m_pURI;
		unsigned int	m_nUriLen;
		RTSP_VERSION	m_version;
        };

        class RtspResponse : public RtspMsgType
        {
        public:
                RtspResponse();
                virtual ~RtspResponse();

                int             Init(RtspMsg *pRequest, RTSP_STATUS_CODE code, const char *pReason);

                inline RTSP_STATUS_CODE GetStatusCode() { return m_code; }
                inline const char *     GetReason() { return m_pReason; }
                inline RTSP_VERSION     GetVersion() { return m_version; }

        protected:

        private:
		RTSP_STATUS_CODE        m_code;
		char		        *m_pReason;
		unsigned int	        m_nReasonLen;
		RTSP_VERSION	        m_version;
        };

        class RtspData : public RtspMsgType
        {
        public:
                RtspData();
                ~RtspData();

                int     Init(char channel);

                inline unsigned char    GetChannel() { return m_szChannel; }

        protected:

        private:
                unsigned char   m_szChannel;
        };

        class RtspMsg : public Msg
        {
        public:
                RtspMsg();

                virtual unsigned int    ProtoID();

                int     Unset();

                int     InitRequest(
                                        RTSP_METHOD     method,
                                        const char      *pUri,
                                        unsigned int    nUriLength);

                int     InitResponse(
                                        RtspMsg                 *pRequest,
                                        RTSP_STATUS_CODE        code,
                                        const char              *pReason);
                
                int     InitData(unsigned char channel, unsigned int nBodySize);
                int     SetBody(unsigned char *pBuf, unsigned int nSize);
                int     SetBodySize(unsigned int nSize);
                inline unsigned int GetBodySize() { return m_nBodySize; }

                RTSP_VERSION    GetVersion();
                int     SetVersion(RTSP_VERSION version);

                RTSP_METHOD     GetMethod();
                int     SetMethod(RTSP_METHOD method);

                int     AddHeader(RTSP_HEADER_FIELD field, const char *pValue, unsigned int nLength);
                int     GetHeader(RTSP_HEADER_FIELD field, char **pValue, int nIndex = 0);
                int     RemoveHeader(RTSP_HEADER_FIELD field, int nIndex);

                virtual bool    ToStringImpl(char **ppbuf, unsigned int *pnSize);

                inline RtspMsgType*     GetMsgType() { return m_pMsgType; }
                inline unsigned char*   GetBody(int *pnBodySize) 
                {
                        *pnBodySize = m_nBodySize;
                        return m_pBody;
                }

                int     GetResponse(RTSP_STATUS_CODE *code, const char **pReason, RTSP_VERSION *pVersion);

        protected:
                virtual ~RtspMsg();
                
                unsigned int            AppendHeaders(char *pBuf/*output*/, unsigned int nBufSize);

        private:
                RtspMsgType             *m_pMsgType;

                RtspKeyValueArray       *m_pHdrFields;

                unsigned char           *m_pBody;
                unsigned int            m_nBodySize;


        };
};

#endif