#include "z3_common.hpp"
#include "z3_rtsp_proto_parser.hpp"
#include "z3_rtsp_ids.hpp"
#include "z3_rtsp_error.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

RtspProtoParser::RtspProtoParser()
        : ProtoParser(RTSP_PROTO_PARSER_ID)
        , m_pRtspMsg(NULL)
        , m_bMsgCompleted(false)
        , m_state(STATE_START)
        , m_nLine(0)
        , m_pBuf(NULL)
        , m_nBufSize(RTSP_RECVBUF_SIZE)
        , m_nBufBytes(0)
        , m_nOffset(0)
{
        m_pRtspMsg = new RtspMsg;

        if (m_nBufSize > 0)
        {
                m_pBuf = (char *)z3_calloc(m_nBufSize, sizeof(char));
                assert(m_pBuf);
        }
}

RtspProtoParser::~RtspProtoParser()
{
        Z3_FREE_POINTER(m_pBuf);
        Z3_OBJ_RELEASE(m_pRtspMsg);
}

int RtspProtoParser::Parse(const char *pBuf, unsigned int nLen, void *pData, Msg **pMsg /* output */)
{
        int             result;
        char            tag, channel, *pos, *hdrval;
        unsigned int    nReadBytes;
        unsigned short  nDataSize;
        long            lHdrLength;
        bool            bBreak;

        if (m_bMsgCompleted)
        {
                m_state         = STATE_START;
                m_nLine         = 0;
                m_nOffset       = 0;
                m_nBufBytes     = 0;

                m_pRtspMsg->Unset();
                m_bMsgCompleted = false;
        }

        if (nLen <= 0 || pBuf == NULL)
                return Z3_ERROR;


        nReadBytes = MIN(m_nBufSize - m_nBufBytes, nLen);

        ::memcpy(m_pBuf + m_nBufBytes, pBuf, nReadBytes);
        m_nBufBytes     += nReadBytes;

        assert(m_nOffset == 0);

        result  = Z3_ERROR;
        bBreak  = false;
        tag     = '\0';

        do
        {
                switch (m_state)
                {
                case STATE_START:
                        tag = *(m_pBuf + m_nOffset);
                        if (tag == '$')
                        {
                                m_state = STATE_DATA_HEADER;
                                
                        }
                        else
                        {
                                m_nLine = 0;
                                m_state = STATE_READ_LINES;
                        }
                        break;

                case STATE_DATA_HEADER:
                        // ensure m_nBufBytes >= 4, '$' + 1 bytes channel + 2 bytes length
                        if ((m_nBufBytes - m_nOffset) >= 4)
                        {
                                channel = *(m_pBuf + m_nOffset + 1);
                                nDataSize = (m_pBuf[m_nOffset + 2] << 8) | m_pBuf[m_nOffset + 3];

                                result = m_pRtspMsg->InitData(channel, nDataSize);
                                if (result == Z3_EOK)
                                {
                                        m_state = STATE_DATA_BODY;
                                        m_nOffset += 4;
                                }
                        }
                        else
                        {
                                result = Z3_RTSP_EINTR;    // wait for more bytes coming
                                bBreak = true;
                        }
                        break;

                case STATE_DATA_BODY:
                        // notice we need to subtract the final '\0'
                        if ((m_nBufBytes - m_nOffset) >= m_pRtspMsg->GetBodySize() - 1)
                        {
                                result = m_pRtspMsg->SetBody((unsigned char *)(m_pBuf + m_nOffset),
                                        m_pRtspMsg->GetBodySize() - 1);
                                m_nOffset += m_pRtspMsg->GetBodySize() - 1; // subtract the final '\0';

                                m_state = STATE_END;
                        }
                        else
                        {
                                result = Z3_RTSP_EINTR;    // wait for more bytes coming
                        }
                        bBreak = true;

                        break;

                case STATE_READ_LINES:
                        tag = '\r';
                        pos = ::strchr(m_pBuf + m_nOffset, tag);
                        if (pos == NULL)
                        {
                                // message is incomplete, continue to wait for more bytes coming
                                result = Z3_RTSP_EINTR;
                                bBreak = true;
                        }
                        else if (pos[1] != '\n')
                        {
                                TRACE_ERROR("Incorrect Rtsp Message: \r\n %s\r\n", pBuf);
                                result = Z3_ERROR;
                                m_state = STATE_END;
                        }
                        else if (pos == m_pBuf + m_nOffset)
                        {
                                // "\r\n\r\n"
                                if (pos[1] == '\n')
                                {
                                        if (Z3_EOK == m_pRtspMsg->GetHeader(RTSP_HDR_CONTENT_LENGTH, &hdrval, 0)
                                                && m_pRtspMsg->GetMsgType()->GetMsgType() ==  RTSP_MSG_RESPONSE)
                                        {
                                                /* there is, prepare to read the body */
                                                lHdrLength = atol (hdrval);
                                                if (lHdrLength > 0)
                                                {
                                                        m_pRtspMsg->SetBodySize(lHdrLength + 1);
                                                        m_state = STATE_DATA_BODY;
                                                }
                                                else
                                                        m_state = STATE_END;
                                        }
                                        else
                                                m_state = STATE_END;

                                        m_nOffset += 2;
                                }
                                else
                                {
                                        // message is incomplete, continue to wait for more bytes coming
                                        result = Z3_RTSP_EINTR;
                                        bBreak = true;
                                }
                        }
                        else
                        {
                                *pos = '\0'; // '\r' change to '\0'
                                
                                pos++;  // move to position '\n'
                                *pos = '\0'; // '\n' change to '\0';

                                NormalizeLine((unsigned char *)m_pBuf + m_nOffset);
                                TrimRightSpace(m_pBuf + m_nOffset);

                                if (m_nLine == 0)
                                {
                                        if (::memcmp(m_pBuf + m_nOffset, "RTSP", 4) != 0)
                                        {
                                                result = ParseRequestLine(m_pBuf + m_nOffset, m_pRtspMsg); 
                                        }
                                        else
                                        {
                                                result = ParseResponseStatus(m_pBuf + m_nOffset, m_pRtspMsg);                                               
                                        }

                                        if (result == Z3_EOK)
                                        {
                                                m_nLine++;
                                                m_nOffset += pos - (m_pBuf + m_nOffset) + 1;
                                        }
                                        else
                                        {
                                                TRACE_ERROR("Error happened when parsing RTSP protocol(%s)\r\n", m_pBuf + m_nOffset);
                                                m_state = STATE_END;
                                        }
                                }
                                else
                                {
                                        result = ParseLine(m_pBuf + m_nOffset, m_pRtspMsg);
                                        if (result == Z3_EOK)
                                        {
                                                m_nOffset += pos - (m_pBuf + m_nOffset) + 1;
                                                m_nLine++;
                                        }
                                        else
                                        {
                                                TRACE_ERROR("Error happened when paring RTSP protocol(%s)\r\n", m_pBuf + m_nOffset);
                                                m_state = STATE_END;
                                        }
                                }
                        }
                        break;
                case STATE_END:

                        bBreak = true;
                        break;
                default:
                        break;
                }

        } while (!bBreak && m_nOffset < m_nBufBytes);


        if (m_nOffset < m_nBufBytes)
        {
                ::memmove(m_pBuf, m_pBuf + m_nOffset, m_nBufBytes - m_nOffset);
                m_nBufBytes -= m_nOffset;
                m_nOffset = 0;
        }

        if (result == Z3_EOK)
        {
                *pMsg = m_pRtspMsg;
                m_bMsgCompleted = true;
        }
        
        return result;
}

int RtspProtoParser::ParseRequestLine(char *pBuf, RtspMsg *pMsg)
{
	int res = Z3_EOK;
	int res2;
	char versionStr[20];
	char methodStr[20];
	char urlStr[4096];
	char *bptr;
	RTSP_METHOD method;
        RTSP_VERSION version;
	
        bptr = (char *)pBuf;

	if (ParseString (methodStr, sizeof (methodStr), &bptr) != Z3_EOK)
		res = Z3_RTSP_EPARSE;

	method = RtspFindMethod(methodStr);

	if (ParseString (urlStr, sizeof (urlStr), &bptr) != Z3_EOK)
		res = Z3_RTSP_EPARSE;

	if (*urlStr == '\0')
		res = Z3_RTSP_EPARSE;

	if (ParseString(versionStr, sizeof (versionStr), &bptr) != Z3_EOK)
		res = Z3_RTSP_EPARSE;

	if (*bptr != '\0')
		res = Z3_RTSP_EPARSE;

        assert(pMsg != NULL);
        if (0 != pMsg->InitRequest(method, urlStr, strlen(urlStr)))
                res = Z3_RTSP_EPARSE;
                
	res2 = ParseProtocolVersion(versionStr, &version);
	if (res == Z3_EOK)
        {
                pMsg->SetVersion(version);
		res = res2;
        }

	/* GET and POST are not allowed as RTSP methods */
	if (pMsg->GetMethod() == RTSP_GET || pMsg->GetMethod() == RTSP_POST)
	{
                pMsg->SetMethod(RTSP_INVALID);
		if (res == Z3_EOK)
			res = Z3_ERROR;
	}

	return res;
}

int RtspProtoParser::ParseLine(char *pBuffer, RtspMsg *pMsg)
{
        RTSP_HEADER_FIELD field;
        char *pLine = pBuffer;
        char *pValue;

        pValue = strchr(pLine, ':');
        if (pValue == NULL || pValue == pLine)
                return Z3_RTSP_EPARSE;

	/* trim space before the colon */
        if (pValue[-1] == ' ')
                pValue[-1] = '\0';

        /* replace the colon with a NUL */
        *pValue++ = '\0';

	/* find the header */
	field = RtspFindHeaderField(pLine);
	if (field == RTSP_HDR_INVALID)
		return Z3_EOK;

        /* split up the value in multiple key:value pairs if it contains comma(s) */
	while (*pValue != '\0')
        {
        	char *pNextValue;
	        char *pComma = NULL;
	        bool quoted = false;
	        unsigned int nComment = 0;

	        /* trim leading space */
	        if (*pValue == ' ')
		        pValue++;

	        /* for headers which may not appear multiple times, and thus may not
	         * contain multiple values on the same line, we can short-circuit the loop
	         * below and the entire value results in just one key:value pair*/
	        if (!RtspHeaderAllowMultiple(field))
		        pNextValue = pValue + strlen (pValue);
	        else
		        pNextValue = pValue;

                /* find the next value, taking special care of quotes and comments */
                while (*pNextValue != '\0')
                {
                        if ((quoted || nComment != 0) && *pNextValue == '\\' && pNextValue[1] != '\0')
                                pNextValue++;
                        else if (nComment == 0 && *pNextValue == '"')
                                quoted = !quoted;
                        else if (!quoted && *pNextValue == '(')
                                nComment++;
                        else if (nComment != 0 && *pNextValue == ')')
                                nComment--;
                        else if (!quoted && nComment == 0) 
                        {
                                /* To quote RFC 2068: "User agents MUST take special care in parsing
	                        * the WWW-Authenticate field value if it contains more than one
	                        * challenge, or if more than one WWW-Authenticate header field is
	                        * provided, since the contents of a challenge may itself contain a
	                        * comma-separated list of authentication parameters."
	                        *
	                        * What this means is that we cannot just look for an unquoted comma
	                        * when looking for multiple values in Proxy-Authenticate and
	                        * WWW-Authenticate headers. Instead we need to look for the sequence
	                        * "comma [space] token space token" before we can split after the
	                        * comma...
	                        */
                                if (field == RTSP_HDR_PROXY_AUTHENTICATE || field == RTSP_HDR_WWW_AUTHENTICATE) 
                                {
	                                if (*pNextValue == ',') 
                                        {
	                                        if (pNextValue[1] == ' ') 
                                                {
	                                                /* skip any space following the comma so we do not mistake it for
	                                                * separating between two tokens */
	                                                pNextValue++;
	                                        }
	                                        pComma = pNextValue;
	                                } 
                                        else if (*pNextValue == ' ' && pNextValue[1] != ',' && pNextValue[1] != '=' && pComma != NULL)
                                        {
	                                        pNextValue = pComma;
	                                        pComma = NULL;
	                                        break;
	                                }
                                } 
                                else if (*pNextValue == ',')
	                                break;
                        }

                        pNextValue++;
                }

                /* trim space */
                if (pValue != pNextValue && pNextValue[-1] == ' ')
                        pNextValue[-1] = '\0';

                if (*pNextValue != '\0')
                        *pNextValue++ = '\0';

                /* add the key:value pair */
                if (*pValue != '\0')
                        pMsg->AddHeader(field, pValue, strlen(pValue) + 1);

                pValue = pNextValue;
	}

        return Z3_EOK;
}

int RtspProtoParser::ParseResponseStatus(char *pBuf, RtspMsg *pMsg)
{
        char            szVersion[20];
        char            szCode[4];
        int             nCode;
        char            *pBegin;
        RTSP_VERSION    version;
        int             result = Z3_EOK;

        pBegin = pBuf;

	if (ParseString(szVersion, sizeof (szVersion), &pBegin) != Z3_EOK)
		result = Z3_RTSP_EPARSE;
        else if (ParseString(szCode, sizeof(szCode), &pBegin) != Z3_EOK)
                result = Z3_RTSP_EPARSE;
        else
        {
                nCode = atoi(szCode);
                if (nCode < 0 || nCode > 600)
                        result = Z3_RTSP_EPARSE;
                else
                {
                        while (isspace(*pBegin))
                                pBegin++;

                        assert(pMsg != NULL);
                        if (pMsg->InitResponse(NULL, nCode, pBegin) != Z3_EOK)
                                result = Z3_RTSP_EPARSE;
                        else
                        {
                                result = ParseProtocolVersion(szVersion, &version);
                                if (result == Z3_EOK)
                                        pMsg->SetVersion(version);

                        }
                }
        }

        return result;
}

int RtspProtoParser::ParseProtocolVersion(char *pszProtocol, RTSP_VERSION *pVersion)
{
        int             result;
        char            dummyChar, *pszVersion;
        unsigned int    nMajorVer, nMinorVer;

        if ((pszVersion = strchr(pszProtocol, '/')) != NULL)
        {
                *pszVersion++ = '\0';

                if (sscanf_s(pszVersion, "%u.%u%c", &nMajorVer, &nMinorVer, &dummyChar) != 2)
                        result = Z3_RTSP_EPARSE;

		if (_stricmp(pszProtocol, "rtsp") == 0)
                {
			if (nMajorVer != 1 || nMinorVer != 0)
                        {
			        *pVersion = RTSP_VERSION_INVALID;
				result = Z3_ERROR;
			}
                        else
                                result = Z3_EOK;
		}
                else
			result = Z3_RTSP_EPARSE;
        }
        else
                result = Z3_RTSP_EPARSE;

        return result;
}