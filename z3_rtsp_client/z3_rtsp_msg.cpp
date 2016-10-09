#include "z3_common.hpp"
#include "z3_rtsp_msg.hpp"
#include "z3_rtsp_error.hpp"

#define RTSP_MSG_STRING_MAX_LENGTH      1500

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

RtspKeyValue::RtspKeyValue()
        : m_field(RTSP_HDR_INVALID)
        , m_nValueLen(0)
        , m_pValue(NULL)
{
        ::memset(m_szInit, 0, sizeof(m_szInit));
}

RtspKeyValue::~RtspKeyValue()
{
        if (m_nValueLen > DEF_MAX_VALUE_LEN)
                Z3_FREE_POINTER(m_pValue)
}

void RtspKeyValue::FillKeyValue(
        RTSP_HEADER_FIELD       Field, 
        const char              *pValue, 
        unsigned int            nValueLen)
{
        if (nValueLen <= DEF_MAX_VALUE_LEN)
        {
                assert(m_pValue == NULL);
                m_pValue = m_szInit;
        }
        else
        {
                assert(m_pValue == NULL);
                m_pValue = (char *)z3_calloc(nValueLen, sizeof(char));
        }

        m_field         = Field;
        m_nValueLen     = nValueLen;
        ::memcpy(m_pValue, pValue, nValueLen);

        return;
}

void RtspKeyValue::UnsetKeyValue()
{
        if (m_pValue != m_szInit)
                z3_free(m_pValue);

        m_field         = RTSP_HDR_INVALID;
        m_nValueLen     = 0;
        m_pValue        = NULL;

        ::memset(m_szInit, 0, sizeof(m_szInit));

        return;
}

RtspKeyValue& RtspKeyValue::operator= (RtspKeyValue &Key)
{
        m_field = Key.m_field;
        m_nValueLen = Key.m_nValueLen;

        if (m_nValueLen > sizeof(m_szInit))
        {
                assert(m_pValue == NULL);
                m_pValue = (char *)z3_malloc(m_nValueLen);
        }
        else
        {
                assert(m_pValue == NULL);
                m_pValue = m_szInit;
        }
        
        ::memcpy(m_pValue, Key.m_pValue, m_nValueLen);

        return *this;
}


RtspKeyValueArray::RtspKeyValueArray()
        : m_nSize(DEF_MAX_KEY_VALUE)
        , m_nKeyValueSum(0)
        , m_pRtspKeyValues(0)
{
        m_pRtspKeyValues = new RtspKeyValue[m_nSize];
}

RtspKeyValueArray::~RtspKeyValueArray()
{
        Z3_DELETE_OBJ_ARRAY(m_pRtspKeyValues);
}

int RtspKeyValueArray::TakeKeyValuesToLarger(unsigned int nLargerSize)
{
        unsigned int nIndex;
        RtspKeyValue *pNewKeyValue;

        if (nLargerSize <= m_nSize)
                return Z3_EINVAL;

        pNewKeyValue = new RtspKeyValue [nLargerSize];
        for (nIndex = 0; nIndex < m_nKeyValueSum; nIndex++)
        {
                pNewKeyValue[nIndex] = m_pRtspKeyValues[nIndex];
        }

        Z3_DELETE_OBJ_ARRAY(m_pRtspKeyValues);

        m_pRtspKeyValues = pNewKeyValue;
        m_nSize = nLargerSize;

        return Z3_EOK;
}

void RtspKeyValueArray::AddKeyValue(
        RTSP_HEADER_FIELD       Field, 
        const char              *pValue, 
        unsigned int            nValueLen)
{
        RtspKeyValue    *pEndKeyValue;

        pEndKeyValue = &(m_pRtspKeyValues[m_nKeyValueSum]);

        pEndKeyValue->FillKeyValue(Field, pValue, nValueLen);

        m_nKeyValueSum++;

        return;
}

void RtspKeyValueArray::PrintEachKeyValue()
{
        RtspKeyValue    *pCurrKeyValue;
        unsigned int    nIndex;

        for (nIndex = 0; nIndex < m_nKeyValueSum; nIndex++)
        {
                pCurrKeyValue = &(m_pRtspKeyValues[nIndex]);
                TRACE_INFO("   key: '%s', value: '%s'\n",
                        RTSP_HEADER_AS_TEXT(pCurrKeyValue->m_field),
                        pCurrKeyValue->m_pValue);
        }
}

RtspKeyValue* RtspKeyValueArray::GetRtspKeyValue(unsigned int nIndex)
{
        if (nIndex < m_nKeyValueSum)
                return &m_pRtspKeyValues[nIndex];
        else
                return NULL;
}

RtspKeyValueArray& RtspKeyValueArray::operator= (RtspKeyValueArray &Array)
{
        unsigned int nIndex;

        m_nSize = Array.m_nSize;
        m_nKeyValueSum = Array.m_nKeyValueSum;

        Z3_DELETE_OBJ_ARRAY(m_pRtspKeyValues);
        
        m_pRtspKeyValues = new RtspKeyValue [m_nSize];
        for (nIndex = 0; nIndex < m_nKeyValueSum; nIndex++)
        {
                m_pRtspKeyValues[nIndex] = Array.m_pRtspKeyValues[nIndex];
        }

        return *this;
}

RtspMsgType::RtspMsgType(RTSP_MSG_TYPE msgType)
        : m_msgType(msgType)
{
}

RtspMsgType::~RtspMsgType()
{
}

RtspRequest::RtspRequest()
        : RtspMsgType(RTSP_MSG_REQUEST)
        , m_method(RTSP_INVALID)
        , m_pURI(NULL)
        , m_nUriLen(0)
        , m_version(RTSP_VERSION_INVALID)
{
}

RtspRequest::~RtspRequest()
{
        Z3_FREE_POINTER(m_pURI);
}

int RtspRequest::Init(RTSP_METHOD method, const char *pUri, unsigned int nUriLength, RTSP_VERSION ver)
{
        int result = Z3_ERROR;

        m_method        = method;
        m_nUriLen       = nUriLength + 1;       
        m_version       = ver;

        if (m_nUriLen > 1)
        {
                Z3_FREE_POINTER(m_pURI);
                m_pURI = (char *)z3_malloc(m_nUriLen);
                if (m_pURI)
                {
                        ::strncpy_s(m_pURI, m_nUriLen, pUri, nUriLength);
                        result = Z3_EOK;
                }
                else
                        result = Z3_ENOMEM;
        }

        return result;
}

RtspResponse::RtspResponse()
        : RtspMsgType(RTSP_MSG_RESPONSE)
        , m_code(RTSP_STS_INVALID)
        , m_pReason(NULL)
        , m_nReasonLen(0)
        , m_version(RTSP_VERSION_INVALID)
{
}

RtspResponse::~RtspResponse()
{
        Z3_FREE_POINTER(m_pReason)
}

int RtspResponse::Init(RtspMsg *pRequest, RTSP_STATUS_CODE code, const char *pReason)
{
        int result = Z3_ERROR;
        
        m_code          = code;
        m_version       = RTSP_VERSION_1_0;

        if (pReason == NULL)
                pReason = (char *)RTSP_STATUS_AS_TEXT(code);
  
        m_nReasonLen = strlen(pReason) + 1;
        if (m_nReasonLen > 1)
        {
                Z3_FREE_POINTER(m_pReason);
                m_pReason = (char *)z3_malloc(m_nReasonLen);
                if (m_pReason)
                {
                        ::strncpy_s(m_pReason, m_nReasonLen, pReason, m_nReasonLen);
                        result = Z3_EOK;
                }
                else
                        result = Z3_ENOMEM;
        }

        return result;
}

RtspData::RtspData()
        : RtspMsgType(RTSP_MSG_DATA)
        , m_szChannel(0xFF)
{
}

RtspData::~RtspData()
{
}

int RtspData::Init(char channel)
{
        m_szChannel = channel;

        return Z3_EOK;
}

RtspMsg::RtspMsg()
        : m_pMsgType(NULL)
        , m_pHdrFields(NULL)
        , m_pBody(NULL)
        , m_nBodySize(0)
{
}

RtspMsg::~RtspMsg()
{
        Z3_DELETE_OBJ(m_pMsgType);
        Z3_DELETE_OBJ(m_pHdrFields);

        Z3_FREE_POINTER(m_pBody);
}

unsigned int RtspMsg::ProtoID()
{
        return RTSP_PROTO_ID;
}

int RtspMsg::Unset()
{
        Z3_DELETE_OBJ(m_pMsgType);
        Z3_DELETE_OBJ(m_pHdrFields);

        Z3_FREE_POINTER(m_pBody);
        m_nBodySize = 0;

        return Z3_EOK;
}

int RtspMsg::InitRequest(RTSP_METHOD method, const char *pUri, unsigned int nUriLength)
{
        RtspRequest *pRequest;
        int result = Z3_ENOMEM;

        Unset();

        assert(m_pMsgType == NULL);
        pRequest = new RtspRequest;
        if (pRequest)
        {                
                result = pRequest->Init(method, pUri, nUriLength);
                if (result == Z3_EOK)
                {
                        m_pMsgType = pRequest;
                }
                else
                {
                        TRACE_ERROR("Failed to init RTSP REQUEST message.");
                        delete pRequest;
                }
        }

        return result;
}

int RtspMsg::InitResponse(RtspMsg *pRequest, RTSP_STATUS_CODE code, const char *pReason)
{
        int     result = Z3_ENOMEM;
        RtspResponse    *pResponse;
        char            *pHeader, *pos;
        unsigned int    nLength;

        Unset();

        assert(m_pMsgType == NULL);
        pResponse = new RtspResponse;
        if (pResponse)
        {
                result = pResponse->Init(pRequest, code, pReason);
                if (0 == result)
                {
                        m_pMsgType = pResponse;
                        if (pRequest)
                        {
                                if (Z3_EOK == pRequest->GetHeader(RTSP_HDR_CSEQ, &pHeader, 0))
                                {
                                        AddHeader(RTSP_HDR_CSEQ, pHeader, strlen(pHeader));
                                }

                                if (Z3_EOK == pRequest->GetHeader(RTSP_HDR_SESSION, &pHeader, 0))
                                {
                                        pHeader = z3_strdup(pHeader);
                                
                                        nLength = strlen(pHeader) + 1;
                                        if ((pos = strchr(pHeader, ':')))
                                                *pos = '\0';

                                        strchomp(pHeader);
                                        result = AddHeader(RTSP_HDR_SESSION, pHeader, strlen(pHeader) + 1);
                                        z3_free(pHeader);
                                }
                        }
                }
                else
                {
                        TRACE_ERROR("Failed to init RTSP REPONDSE message.");
                        delete pResponse;
                }

        }

        return result;
}

RTSP_VERSION RtspMsg::GetVersion()
{
        RTSP_VERSION version = RTSP_VERSION_INVALID;

        if (m_pMsgType == NULL)
                return RTSP_VERSION_INVALID;

        switch (m_pMsgType->GetMsgType())
        {
        case RTSP_MSG_REQUEST:
                RtspRequest *pRequest;
                pRequest = dynamic_cast<RtspRequest *>(m_pMsgType);
                version = pRequest->GetVersion();
                break;

        case RTSP_MSG_RESPONSE:
        default:
                break;
        }

        return version;
}

int RtspMsg::SetVersion(RTSP_VERSION version)
{
        int result = Z3_ERROR;

        if (m_pMsgType == NULL)
                return Z3_ERROR;

        switch (m_pMsgType->GetMsgType())
        {
        case RTSP_MSG_REQUEST:
                RtspRequest *pRequest;

                pRequest = dynamic_cast<RtspRequest *>(m_pMsgType);
                pRequest->SetVersion(version);
                result = Z3_EOK;
                break;

        case RTSP_MSG_RESPONSE:
        default:
                break;
        }

        return result;
}

RTSP_METHOD RtspMsg::GetMethod()
{
        RTSP_METHOD method = RTSP_INVALID;

        if (m_pMsgType == NULL)
                return RTSP_INVALID;

        switch (m_pMsgType->GetMsgType())
        {
        case RTSP_MSG_REQUEST:
                RtspRequest *pRequest;
                pRequest = dynamic_cast<RtspRequest *>(m_pMsgType);
                method = pRequest->GetMethod();
                break;
        case RTSP_MSG_RESPONSE:
        default:
                break;
        }

        return method;
}

int RtspMsg::SetMethod(RTSP_METHOD method)
{
        int result = Z3_ERROR;

        if (m_pMsgType == NULL)
                return Z3_ERROR;

        switch (m_pMsgType->GetMsgType())
        {
        case RTSP_MSG_REQUEST:
                RtspRequest *pRequest;

                pRequest = dynamic_cast<RtspRequest *>(m_pMsgType);
                pRequest->SetMethod(method);
                result = Z3_EOK;
                break;

        case RTSP_MSG_RESPONSE:
        default:
                break;
        }

        return result;
}

int RtspMsg::AddHeader(
        RTSP_HEADER_FIELD       field,
        const char              *pValue,
        unsigned int            nLength)
{
        RtspKeyValueArray *pKeyValues;

        if (m_pHdrFields == NULL)
                m_pHdrFields = new RtspKeyValueArray;

        pKeyValues = m_pHdrFields;
        if (pKeyValues->Size() == DEF_MAX_KEY_VALUE)
        {
                if (pKeyValues->KeyValueSum() == pKeyValues->Size())
                        if (pKeyValues->TakeKeyValuesToLarger(TOP_MAX_KEY_VALUE) != Z3_EOK)
                                return Z3_ERROR;

                pKeyValues->AddKeyValue(field, pValue, nLength);

                return Z3_EOK;
        }
        else if (pKeyValues->Size() == TOP_MAX_KEY_VALUE)
        {
                if (pKeyValues->KeyValueSum() == pKeyValues->Size())
                        return Z3_ERROR;

                pKeyValues->AddKeyValue(field, pValue, nLength);
                
                return Z3_EOK;
        }

        return Z3_ERROR;
}

int RtspMsg::GetHeader(RTSP_HEADER_FIELD field, char **pValue, int nIndex)
{
        RtspKeyValue *pKeyValue;
        unsigned int i, nCnt = 0;

        if (m_pHdrFields == NULL)
                return Z3_RTSP_ENOTIMPL;

        for (i = 0; i < m_pHdrFields->KeyValueSum(); i++)
        {
                pKeyValue = m_pHdrFields->GetRtspKeyValue(i);
                if (pKeyValue->m_field == field && nCnt++ == nIndex)
                {
                        if (pValue)
                        {
                                *pValue = pKeyValue->m_pValue;
                                return Z3_EOK;
                        }
                }

        }

        return Z3_RTSP_ENOTIMPL;
}

int RtspMsg::RemoveHeader(RTSP_HEADER_FIELD field, int nIndex)
{
        unsigned int    i, cnt, nLastNum;
        RtspKeyValue    *pKeyValue, *pLastKeyValue;
        int     result = Z3_RTSP_ENOTIMPL;

        if (m_pHdrFields == NULL)
                return Z3_EOK;

        cnt = 0;
        for (i = 0; i < m_pHdrFields->KeyValueSum(); i++)
        {
                pKeyValue = m_pHdrFields->GetRtspKeyValue(i);
                if (pKeyValue->m_field == field && (nIndex == -1 || cnt++ == nIndex))
                {
                        pKeyValue->UnsetKeyValue();

                        nLastNum = m_pHdrFields->KeyValueSum() - 1;
                        if (i < nLastNum)
                        {
                                /* 把最后一个key_value拷贝到移除的key_value的位置 */
                                pLastKeyValue = m_pHdrFields->GetRtspKeyValue(i);
                                pKeyValue->FillKeyValue(pLastKeyValue->m_field, pLastKeyValue->m_pValue, pLastKeyValue->m_nValueLen);

                                pLastKeyValue->UnsetKeyValue();
                        }

                        m_pHdrFields->KeyValueSumDecrease();

                        result = Z3_EOK;
                        break;
                }
        }

        return result;
}

int RtspMsg::InitData(unsigned char channel, unsigned int nBodySize)
{
        int result;
        RtspData *pData;

        Unset();

        result = Z3_ERROR;
        assert(m_pMsgType == NULL);

        pData = new RtspData;
        if (pData)
        {
                pData->Init(channel);
                m_pBody = (unsigned char *)z3_calloc(nBodySize, sizeof(char));
                if (m_pBody)
                {
                        m_nBodySize = nBodySize;
                        result = Z3_EOK;
                }
        }

        return result;
}

int RtspMsg::SetBody(unsigned char *pBuf, unsigned int nSize)
{
        assert(m_pBody != NULL);
        assert(m_nBodySize > nSize);

        memcpy(m_pBody, pBuf, nSize);

        return Z3_EOK;
}

int RtspMsg::SetBodySize(unsigned int nSize)
{
        if (nSize > 0)
        {
                assert(m_pBody == NULL);
                assert(m_nBodySize == 0);

                m_pBody = (unsigned char *)z3_calloc(nSize, sizeof(unsigned char));
                if (m_pBody)
                {
                        m_nBodySize = nSize;
                        return Z3_EOK;
                }

        }

        return Z3_ENOMEM;
}

bool RtspMsg::ToStringImpl(char **ppBuf, unsigned int *pnSize)
{
        char            *pString;
        RTSP_MSG_TYPE   msgType;
        int             nCount, nOffset = 0;

        pString = (char *)z3_malloc(RTSP_MSG_STRING_MAX_LENGTH);
        if (pString == NULL)
        {
                TRACE_ERROR("memory is not enough");
                return false;
        }

        msgType = m_pMsgType->GetMsgType();
        switch (msgType)
        {
        case RTSP_MSG_REQUEST:
                {
                        RtspRequest *pRequest = dynamic_cast<RtspRequest *>(m_pMsgType);
                        nCount = _snprintf_s(pString, RTSP_MSG_STRING_MAX_LENGTH, RTSP_MSG_STRING_MAX_LENGTH, "%s %s RTSP/1.0\r\n",
                                        RTSP_METHOD_AS_TEXT(pRequest->GetMethod()),
                                        pRequest->GetUri());
                        if (nCount > 0)
                                nOffset += nCount;
                        else
                        {
                                TRACE_ERROR("Failed to generate RTSP request string.");
                                return false;
                        }
                        break;
                }
        case RTSP_MSG_RESPONSE:
                {
                        RtspResponse *pResponse = dynamic_cast<RtspResponse *>(m_pMsgType);
                        nCount = _snprintf_s(pString, RTSP_MSG_STRING_MAX_LENGTH, RTSP_MSG_STRING_MAX_LENGTH, "RTSP/1.0 %d %s\r\n",
                                        pResponse->GetStatusCode(), 
                                        pResponse->GetReason());
                        if (nCount > 0)
                                nOffset += nCount;
                        else
                        {
                                TRACE_ERROR("Failed to generate RTSP response string.");
                                return false;
                        }
                        break;
                }
        case RTSP_MSG_DATA:
                {
                        RtspData *pData = dynamic_cast<RtspData *>(m_pMsgType);

                        if (RTSP_MSG_STRING_MAX_LENGTH <= m_nBodySize + 4)
                        {
                                TRACE_ERROR("memory is not enough");
                                return false;
                        }

                        pString[0] = '$';
                        pString[1] = pData->GetChannel();
                        pString[2] = (m_nBodySize >> 8) & 0xff;
                        pString[3] = m_nBodySize & 0xff;

                        memcpy(pString + 4, m_pBody, m_nBodySize);
                        nOffset += 4 + m_nBodySize;

                        break;
                }

        default:
                assert(false);
                break;
        }

        if (msgType != RTSP_MSG_DATA)
        {
                char dateString[128];

                if (msgType == RTSP_MSG_RESPONSE)
                {
                        GenerateDateString(dateString, sizeof(dateString));
                        RemoveHeader(RTSP_HDR_DATE, -1);
                        AddHeader(RTSP_HDR_DATE, dateString, strlen(dateString)+1);
                }

                RemoveHeader(RTSP_HDR_CONTENT_LENGTH, -1);
                if (m_pBody != NULL && m_nBodySize > 0)
                {
                        assert(false); // 要检查下一句
                        sprintf_s(dateString, sizeof(dateString), "%d", m_nBodySize);
                        AddHeader(RTSP_HDR_CONTENT_LENGTH, dateString, strlen(dateString) + 1);
                }

                if (nOffset < RTSP_MSG_STRING_MAX_LENGTH)
                {
                        nCount = AppendHeaders(pString + nOffset, RTSP_MSG_STRING_MAX_LENGTH - nOffset);
                        if (nCount > 0)
                                nOffset += nCount;
                        else
                        {
                                TRACE_ERROR("Failed to generate header string of RTSP message.");
                                return false;
                        }
                }

                nCount = _snprintf_s(pString + nOffset, RTSP_MSG_STRING_MAX_LENGTH - nOffset, RTSP_MSG_STRING_MAX_LENGTH - nOffset, "\r\n");
                assert(nCount > 0);
                nOffset += nCount;

                if (m_pBody && m_nBodySize > 0)
                {
                        nCount = _snprintf_s(pString + nOffset, RTSP_MSG_STRING_MAX_LENGTH - nOffset, RTSP_MSG_STRING_MAX_LENGTH - nOffset, "%s", m_pBody);
                        if (nCount > 0)
                                nOffset += nCount;
                        else
                        {
                                TRACE_ERROR("Failed to generate header string of RTSP message.");
                                return false;
                        }
                }
        }

        *(pString + nOffset) = '\0';
        *pnSize = nOffset;
        *ppBuf = pString;

        return true;
}

unsigned int RtspMsg::AppendHeaders(char *pBuf/*output*/, unsigned int nBufSize)
{
        RtspKeyValue    *pValue;
        unsigned int    nIndex, nSize, nTotal;

        nTotal = 0;
        for (nIndex = 0; nIndex < m_pHdrFields->KeyValueSum(); nIndex++)
        {
                pValue = m_pHdrFields->GetRtspKeyValue(nIndex);
                nSize = _snprintf_s(pBuf, nBufSize, nBufSize, "%s: %s\r\n", 
                                RTSP_HEADER_AS_TEXT(pValue->m_field),
                                pValue->m_pValue);
                if (nSize <= 0)
                {
                        TRACE_ERROR("Failed to convert RTSP headers to string");
                        return 0;
                }

                if (nBufSize > nSize)
                {
                        nTotal   += nSize;
                        nBufSize -= nSize;                
                        pBuf     += nSize;
                }
                else
                {
                        TRACE_ERROR("Error! Pointer pBuf(0x%p) shall be overflow!");
                        return 0;
                }                
        }

        *pBuf = '\0';

        return nTotal;
}

int RtspMsg::GetResponse(
        RTSP_STATUS_CODE        *code,
        const char              **pReason,
        RTSP_VERSION            *pVersion)
{
        RtspResponse    *pResponse;

        if (m_pMsgType->GetMsgType() != RTSP_MSG_RESPONSE)
                return Z3_ERROR;

        pResponse = dynamic_cast<RtspResponse *>(m_pMsgType);
        *code = pResponse->GetStatusCode();
        *pReason = pResponse->GetReason();
        *pVersion = pResponse->GetVersion();

        return Z3_EOK;
}