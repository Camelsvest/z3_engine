#include "z3_common.hpp"
#include "z3_sdp.hpp"
#include "z3_rtsp_def.hpp"

using namespace Z3;

//#ifdef Z3_MEM_DEBUG
//#define new z3_debug_new
//#endif

enum
{
        SDP_SESSION,
        SDP_MEDIA,
};


SDPBandwidth::SDPBandwidth()
        : m_pszBWType(NULL)
        , m_nBandwidth(0)
{
}

SDPBandwidth::~SDPBandwidth()
{
        Z3_FREE_POINTER(m_pszBWType);
}

bool SDPBandwidth::SetBandwidth(char *pszType, unsigned int nBandwidth)
{
        Z3_FREE_POINTER(m_pszBWType);

        m_pszBWType = z3_strdup(pszType);
        if (m_pszBWType)
        {
                m_nBandwidth = nBandwidth;
                return true;
        }

        return false;
}

SDPTime::SDPTime()
        : m_pszStart(NULL)
        , m_pszStop(NULL)
        , m_pRepeat(NULL)
{
}

SDPTime::~SDPTime()
{
        STL_STRING_VECTOR::iterator itera;
        STL_STRING *pString;

        Z3_FREE_POINTER(m_pszStart);
        Z3_FREE_POINTER(m_pszStop);

        assert(m_pRepeat); // to do !!!
        for (itera = m_pRepeat->begin(); itera != m_pRepeat->end(); itera++)
        {
                pString = *itera;
                Z3_DELETE_OBJ(pString);
        }

        Z3_DELETE_OBJ(m_pRepeat);
}

SDPZone::SDPZone()
        : m_pszTime(NULL)
        , m_pszTypedTime(NULL)
{
}

SDPZone::~SDPZone()
{
        Z3_FREE_POINTER(m_pszTime);
        Z3_FREE_POINTER(m_pszTypedTime);
}

SDPOrigin::SDPOrigin()
        : m_pszUsername(NULL)
        , m_pszSessId(NULL)
        , m_pszSessVersion(NULL)
        , m_pszNetType(NULL)
        , m_pszAddrType(NULL)
        , m_pszAddr(NULL)
{
}

SDPOrigin::~SDPOrigin()
{
        Z3_FREE_POINTER(m_pszUsername);
        Z3_FREE_POINTER(m_pszSessId);
        Z3_FREE_POINTER(m_pszSessVersion);        
        Z3_FREE_POINTER(m_pszNetType);
        Z3_FREE_POINTER(m_pszAddrType);
        Z3_FREE_POINTER(m_pszAddr);
}

int SDPOrigin::SetField(char **ppDest, char *pSrc)
{
        Z3_FREE_POINTER(*ppDest);
        *ppDest = z3_strdup(pSrc);

        return strlen(*ppDest);        
}

int SDPOrigin::SetUsername(char *pszUserName)
{
        return SetField(&m_pszUsername, pszUserName);
}

char* SDPOrigin::GetUsername()
{
        return m_pszUsername;
}

int SDPOrigin::SetSessId(char *pszSessId)
{
        return SetField(&m_pszSessId, pszSessId);
}

char* SDPOrigin::GetSessId()
{
        return m_pszSessId;
}

int SDPOrigin::SetSessVersion(char *pszSessVersion)
{
        return SetField(&m_pszSessVersion, pszSessVersion);
}

char* SDPOrigin::GetSessVersion()
{
        return m_pszSessVersion;
}

int SDPOrigin::SetNetType(char *pszNetType)
{
        return SetField(&m_pszNetType, pszNetType);
}

char* SDPOrigin::GetNetType()
{
        return m_pszNetType;
}

int SDPOrigin::SetAddrType(char *pszAddrType)
{
        return SetField(&m_pszAddrType, pszAddrType);
}

char* SDPOrigin::GetAddrType()
{
        return m_pszAddrType;
}

int SDPOrigin::SetAddr(char *pszAddr)
{
        return SetField(&m_pszAddr, pszAddr);
}

char* SDPOrigin::GetAddr()
{
        return m_pszAddr;
}

SDPConnection::SDPConnection()
        : m_pszNetType(NULL)
        , m_pszAddrType(NULL)
        , m_pszAddress(NULL)
        , m_nTTL(0)
        , m_nAddrNum(0)
{
}

SDPConnection::~SDPConnection()
{
        Z3_FREE_POINTER(m_pszNetType);
        Z3_FREE_POINTER(m_pszAddrType);
        Z3_FREE_POINTER(m_pszAddress);
}

SDPConnection& SDPConnection::operator = (SDPConnection &conn)
{
        Z3_FREE_POINTER(m_pszNetType);
        m_pszNetType = z3_strdup(conn.m_pszNetType);

        Z3_FREE_POINTER(m_pszAddrType);
        m_pszAddrType = z3_strdup(conn.m_pszAddrType);

        Z3_FREE_POINTER(m_pszAddress);
        m_pszAddress = z3_strdup(conn.m_pszAddress);

        m_nTTL = conn.m_nTTL;
        m_nAddrNum = conn.m_nAddrNum;

        return *this;
}

int SDPConnection::SetNetType(char *pszNetType)
{
        Z3_FREE_POINTER(m_pszNetType);

        m_pszNetType = z3_strdup(pszNetType);

        return strlen(m_pszNetType);
}

int SDPConnection::SetAddrType(char *pszAddrType)
{
        Z3_FREE_POINTER(m_pszAddrType);

        m_pszAddrType = z3_strdup(pszAddrType);

        return strlen(m_pszAddrType);
}

const char * SDPConnection::GetAddrType()
{
        return m_pszAddrType;
}

int SDPConnection::SetAddress(char *pszAddress)
{
        Z3_FREE_POINTER(m_pszAddress);

        m_pszAddress = z3_strdup(pszAddress);

        return strlen(m_pszAddress);
}

void SDPConnection::SetTTL(unsigned int nTtl)
{
        m_nTTL = nTtl;
}

void SDPConnection::SetAddrNum(unsigned int num)
{
        m_nAddrNum = num;
}

SDPKey::SDPKey()
        : m_pszType(NULL)
        , m_pszData(NULL)
{
}

SDPKey::~SDPKey()
{
        Z3_FREE_POINTER(m_pszType);
        Z3_FREE_POINTER(m_pszData);
}

SDPAttribute::SDPAttribute()
        : m_pszKey(NULL)
        , m_pszValue(NULL)
{
}

SDPAttribute::~SDPAttribute()
{
        Z3_FREE_POINTER(m_pszKey);
        Z3_FREE_POINTER(m_pszValue);
}

bool SDPAttribute::SetAttribute(char *pszKey, char *pszValue)
{
        Z3_FREE_POINTER(m_pszKey);
        m_pszKey = z3_strdup(pszKey);

        Z3_FREE_POINTER(m_pszValue);
        m_pszValue = z3_strdup(pszValue);

        if (m_pszKey && m_pszValue)
                return true;
        else
                return false;
}

SDPMedia::SDPMedia()
        : m_pszMedia(NULL)
        , m_nPort(0)
        , m_nPortNum(0xFFFFFFFF)
        , m_pszProto(NULL)
        , m_pVectorFmts(NULL)
        , m_pszInfo(NULL)
        , m_pVectorConnections(NULL)
        , m_pVectorBandwidths(NULL)
        , m_pVectorAttributes(NULL)
{
}

SDPMedia::~SDPMedia()
{
        STL_STRING_VECTOR::iterator             itera;
        STL_STRING                              *pString;
        std::vector<SDPConnection *>::iterator  iteraConn;
        SDPConnection                           *pConnection;
        std::vector<SDPBandwidth *>::iterator   iteraBandwidth;
        SDPBandwidth                            *pBandwidth;
        std::vector<SDPAttribute *>::iterator   iteraAttribute;
        SDPAttribute                            *pAttribute;

        Z3_FREE_POINTER(m_pszMedia);
        Z3_FREE_POINTER(m_pszProto);
        Z3_FREE_POINTER(m_pszInfo);

        if (m_pVectorFmts)
        {
                for (itera = m_pVectorFmts->begin(); itera != m_pVectorFmts->end(); itera++)
                {
                        pString = *itera;
                        Z3_DELETE_OBJ(pString);
                }
                Z3_DELETE_OBJ(m_pVectorFmts);
        }

        if (m_pVectorConnections)
        {
                for (iteraConn = m_pVectorConnections->begin(); iteraConn != m_pVectorConnections->end(); iteraConn++)
                {
                        pConnection = *iteraConn;
                        Z3_DELETE_OBJ(pConnection);
                }
                Z3_DELETE_OBJ(m_pVectorConnections);
        }

        if (m_pVectorBandwidths)
        {
                for (iteraBandwidth = m_pVectorBandwidths->begin(); iteraBandwidth != m_pVectorBandwidths->end(); iteraBandwidth++)
                {
                        pBandwidth = *iteraBandwidth;
                        Z3_DELETE_OBJ(pBandwidth);
                }
                Z3_DELETE_OBJ(m_pVectorBandwidths);
        }

        if (m_pVectorAttributes)
        {
                for (iteraAttribute = m_pVectorAttributes->begin(); iteraAttribute != m_pVectorAttributes->end(); iteraAttribute++)
                {
                        pAttribute = *iteraAttribute;
                        Z3_DELETE_OBJ(pAttribute);
                }
                Z3_DELETE_OBJ(m_pVectorAttributes);
        }
}

bool SDPMedia::SetMedia(char *pszMedia)
{
        Z3_FREE_POINTER(m_pszMedia);

        m_pszMedia = z3_strdup(pszMedia);

        return (m_pszMedia != NULL) ? true : false;
}

const char * SDPMedia::GetMedia()
{
        return m_pszMedia;
}

void SDPMedia::SetSDPort(unsigned int nPort)
{
        m_nPort = nPort;
}

void SDPMedia::SetPortNum(unsigned int nPortNum)
{
        m_nPortNum = nPortNum;
}

bool SDPMedia::SetProtocol(char *pszProtocol)
{
        Z3_FREE_POINTER(m_pszProto);
        m_pszProto = z3_strdup(pszProtocol);

        return (m_pszProto != NULL) ? true : false;
}

bool SDPMedia::AddFormat(char *pszFormat)
{
        if (m_pVectorFmts == NULL)
                m_pVectorFmts = new STL_STRING_VECTOR;

        if (m_pVectorFmts)
        {
                STL_STRING *pstrFormat;

                pstrFormat = new STL_STRING;
                if (pstrFormat)
                {
                        pstrFormat->append(pszFormat);
                        m_pVectorFmts->push_back(pstrFormat);

                        return true;
                }
        }

        return false;
}

bool SDPMedia::AddConnection(SDPConnection *pConnection)
{
        if (m_pVectorConnections == NULL)
                m_pVectorConnections = new std::vector<SDPConnection *>;

        if (m_pVectorConnections)
        {
                m_pVectorConnections->push_back(pConnection);
                return true;
        }

        return false;
}

bool SDPMedia::AddBandwidth(char *pszType, char *pszBandwidth)
{
        if (m_pVectorBandwidths == NULL)
                m_pVectorBandwidths = new std::vector<SDPBandwidth *>;

        if (m_pVectorBandwidths)
        {
                SDPBandwidth *pBandwidth;

                pBandwidth = z3_debug_new SDPBandwidth;
                if (pBandwidth)
                {
                        m_pVectorBandwidths->push_back(pBandwidth);
                        return pBandwidth->SetBandwidth(pszType, atoi(pszBandwidth));
                }
        }

        return false;
}

bool SDPMedia::AddAttribute(char *pszKey, char *pszValue)
{
        if (m_pVectorAttributes == NULL)
                m_pVectorAttributes = new std::vector<SDPAttribute *>;

        if (m_pVectorAttributes)
        {
                SDPAttribute *pAttribute;

                pAttribute = new SDPAttribute;
                if (pAttribute)
                {
                        m_pVectorAttributes->push_back(pAttribute);
                        return pAttribute->SetAttribute(pszKey, pszValue);
                }
        }

        return false;
}

char * SDPMedia::GetAttributeValue(char *pszKey, unsigned int nth)
{
        uint32_t         nIndex;
        SDPAttribute    *pAttribute;

        if (m_pVectorAttributes)
        {
                for (nIndex = 0; nIndex < m_pVectorAttributes->size(); nIndex++)
                {
                        pAttribute = m_pVectorAttributes->at(nIndex);
                        if (_stricmp(pAttribute->GetKey(), pszKey) == 0)
                        {
                                if (nth == 0)
                                        return pAttribute->GetValue();
                                else
                                        nth--;
                        }

                }
        }

        return NULL;
}

int SDPMedia::SetInfo(char *pszInfo)
{
        Z3_FREE_POINTER(m_pszInfo);

        m_pszInfo = z3_strdup(pszInfo);

        return strlen(m_pszInfo);
}

SDPMessage::SDPMessage()
        : m_pszVersion(NULL)
        , m_pszSessionName(NULL)
        , m_pszInfo(NULL)
        , m_pszUrl(NULL)
        , m_pVectorEmails(NULL)
        , m_pVectorPhones(NULL)
        , m_pConnection(NULL)
        , m_pVectorBandwidths(NULL)
        , m_pVectorTimes(NULL)
        , m_pVectorZones(NULL)
        , m_pVectorAttributes(NULL)
        , m_pVectorMedias(NULL)
{
}

SDPMessage::~SDPMessage()
{
        STL_STRING_VECTOR::iterator             iteraString;
        STL_STRING                              *pString;
        std::vector<SDPBandwidth *>::iterator   iteraBandwidth;
        SDPBandwidth                            *pBandwidth;
        std::vector<SDPAttribute *>::iterator   iteraAttribute;
        SDPAttribute                            *pAttribute;
        std::vector<SDPTime *>::iterator        iteraTime;
        SDPTime                                 *pTime;
        std::vector<SDPZone *>::iterator        iteraZone;
        SDPZone                                 *pZone;
        std::vector<SDPMedia *>::iterator       iteraMedia;
        SDPMedia                                *pMedia;

        Z3_FREE_POINTER(m_pszVersion);
        Z3_FREE_POINTER(m_pszSessionName);
        Z3_FREE_POINTER(m_pszInfo);
        Z3_FREE_POINTER(m_pszUrl);

        if (m_pVectorEmails)
        {
                for (iteraString = m_pVectorEmails->begin(); iteraString != m_pVectorEmails->end(); iteraString++)
                {
                        pString = *iteraString;
                        Z3_DELETE_OBJ(pString);
                }
                Z3_DELETE_OBJ(m_pVectorEmails);
        }

        if (m_pVectorPhones)
        {
                for (iteraString = m_pVectorPhones->begin(); iteraString != m_pVectorPhones->end(); iteraString++)
                {
                        pString = *iteraString;
                        Z3_DELETE_OBJ(pString);
                }
                Z3_DELETE_OBJ(m_pVectorPhones);
        }

        Z3_DELETE_OBJ(m_pConnection);

        if (m_pVectorBandwidths)
        {
                for (iteraBandwidth = m_pVectorBandwidths->begin(); iteraBandwidth != m_pVectorBandwidths->end(); iteraBandwidth++)
                {
                        pBandwidth = *iteraBandwidth;
                        Z3_DELETE_OBJ(pBandwidth);
                }
                Z3_DELETE_OBJ(m_pVectorBandwidths);
        }

        if (m_pVectorTimes)
        {
                for (iteraTime = m_pVectorTimes->begin(); iteraTime != m_pVectorTimes->end(); iteraTime++)
                {
                        pTime = *iteraTime;
                        Z3_DELETE_OBJ(pTime);
                }
                Z3_DELETE_OBJ(m_pVectorTimes);
        }

        if (m_pVectorZones)
        {
                for (iteraZone = m_pVectorZones->begin(); iteraZone != m_pVectorZones->end(); iteraZone++)
                {
                        pZone = *iteraZone;
                        Z3_DELETE_OBJ(pZone);
                }
                Z3_DELETE_OBJ(m_pVectorZones);
        }

        if (m_pVectorAttributes)
        {
                for (iteraAttribute = m_pVectorAttributes->begin(); iteraAttribute != m_pVectorAttributes->end(); iteraAttribute++)
                {
                        pAttribute = *iteraAttribute;
                        Z3_DELETE_OBJ(pAttribute);
                }
                Z3_DELETE_OBJ(m_pVectorAttributes);
        }

        if (m_pVectorMedias)
        {
                for (iteraMedia = m_pVectorMedias->begin(); iteraMedia != m_pVectorMedias->end(); iteraMedia++)
                {
                        pMedia = *iteraMedia;
                        Z3_DELETE_OBJ(pMedia);
                }
                Z3_DELETE_OBJ(m_pVectorMedias);
        }
}

int SDPMessage::SetVersion(char *pszVersion)
{
        Z3_FREE_POINTER(m_pszVersion);

        m_pszVersion = z3_strdup(pszVersion);
        
        return strlen(m_pszVersion);
}

const char * SDPMessage::GetVersion()
{
        return m_pszVersion;
}

SDPOrigin& SDPMessage::GetOrigin()
{
        return m_origin;
}

int SDPMessage::SetSessionName(char *pszSessionName)
{
        Z3_FREE_POINTER(m_pszSessionName);

        m_pszSessionName = z3_strdup(pszSessionName);

        return strlen(m_pszSessionName);
}

const char * SDPMessage::GetSessionName()
{
        return m_pszSessionName;
}

int SDPMessage::SetInfo(char *pszInfo)
{
        Z3_FREE_POINTER(m_pszInfo);

        m_pszInfo = z3_strdup(pszInfo);

        return strlen(m_pszInfo);
}

const char * SDPMessage::GetInfo()
{
        return m_pszInfo;
}

int SDPMessage::SetUrl(char *pszUrl)
{
        Z3_FREE_POINTER(m_pszUrl);

        m_pszUrl = z3_strdup(pszUrl);

        return strlen(m_pszUrl);
}

const char * SDPMessage::GetUrl()
{
        return m_pszUrl;
}

bool SDPMessage::AddItemIntoVector(STL_STRING_VECTOR *pVector, char *pszItem)
{
        STL_STRING     *pstr;

        if (pVector)
        {
                pstr = new STL_STRING;
                pstr->append(pszItem);

                pVector->push_back(pstr);

                return true;
        }

        return false;
}

bool SDPMessage::AddMail(char *pszMail)
{
        if (m_pVectorEmails == NULL)
                m_pVectorEmails = new STL_STRING_VECTOR;

        if (m_pVectorEmails)
                return AddItemIntoVector(m_pVectorEmails, pszMail);
        else
                return false;
}

bool SDPMessage::AddPhone(char *pszPhone)
{
        if (m_pVectorPhones == NULL)
                m_pVectorPhones = new STL_STRING_VECTOR;

        return AddItemIntoVector(m_pVectorPhones, pszPhone);
}

bool SDPMessage::AddBandwidth(char *pszType, char *pszBandwidth)
{
        if (m_pVectorBandwidths == NULL)
                m_pVectorBandwidths = new std::vector<SDPBandwidth *>;

        if (m_pVectorBandwidths)
        {
                SDPBandwidth *pBandwidth;

                pBandwidth = new SDPBandwidth;
                if (pBandwidth)
                        return pBandwidth->SetBandwidth(pszType, atoi(pszBandwidth));
        }

        return false;
}

bool SDPMessage::AddAttribute(char *pszKey, char *pszValue)
{
        if (m_pVectorAttributes == NULL)
                m_pVectorAttributes = new std::vector<SDPAttribute *>;

        if (m_pVectorAttributes)
        {
                SDPAttribute *pAttribute;

                pAttribute = z3_debug_new SDPAttribute;
                if (pAttribute)
                {
                        m_pVectorAttributes->push_back(pAttribute);
                        return pAttribute->SetAttribute(pszKey, pszValue);
                }
        }

        return false;
}

unsigned int SDPMessage::GetMediaCount()
{
        if (m_pVectorMedias)
                return m_pVectorMedias->size();
        
        return 0;
}

bool SDPMessage::AddMedia(SDPMedia *pMedia)
{
        if (m_pVectorMedias == NULL)
                m_pVectorMedias = new std::vector<SDPMedia *>;

        if (m_pVectorMedias)
        {
                m_pVectorMedias->push_back(pMedia);
                return true;
        }

        return false;
}

SDPMedia* SDPMessage::GetSDPMedia(unsigned int nIndex)
{
        assert(m_pVectorMedias);
        assert(nIndex >= 0 && nIndex < m_pVectorMedias->size());

        return m_pVectorMedias->at(nIndex);
}

void SDPMessage::SetConnection(SDPConnection *pConn)
{
        m_pConnection = pConn;
}

int SDPMessage::ParseString(const char *pszBuf, unsigned int nBufSize)
{
        char            *pos, type;
        unsigned int    idx;
        SDPContext      ctx;
        char            buffer[1024];

        if (pszBuf == NULL || nBufSize <= 0)
                return EINVAL;

        ctx.state = SDP_SESSION;
        ctx.media = NULL;
        ctx.msg = this;

        pos = (char *)pszBuf;
        while (true)
        {
                while (isspace(*pos))
                        pos++;

                type = *pos++;
                if (type == '\0')
                        break;

                if (*pos != '=')
                        goto line_done;
                pos++;

                idx = 0;
                while (*pos != '\n' && *pos != '\r' && *pos != '\0')
                {
                        if (idx < sizeof (buffer) - 1)
                                buffer[idx++] = *pos;

                        pos++;
                }
                buffer[idx] = '\0';
                ParseLine (&ctx, type, buffer);

line_done:
                while (*pos != '\n' && *pos != '\0')
                        pos++;

                if (*pos == '\n')
                        pos++;
        }

        return 0;
}

int SDPMessage::ParseLine(SDPContext *pCtx, char type, char *pszLine)
{
        char str[1024];
        char *p = pszLine;
        SDPOrigin &origin = pCtx->msg->GetOrigin();

        switch (type)
        {
        case 'v':
                if (pszLine[0] != '0')
                        TRACE_WARN("wrong SDP version");
                pCtx->msg->SetVersion(pszLine);
                break;
        case 'o':
                read_string(str, sizeof(str), &p);
                origin.SetUsername(str);
                        
                read_string(str, sizeof(str), &p);
                origin.SetSessId(str);
                        
                read_string(str, sizeof(str), &p);
                origin.SetSessVersion(str);

                read_string(str, sizeof(str), &p);
                origin.SetNetType(str);
                        
                read_string(str, sizeof(str), &p);
                origin.SetAddrType(str);
                        
                read_string(str, sizeof(str), &p);
                origin.SetAddr(str);

                break;
        case 's':
                pCtx->msg->SetSessionName(pszLine);
                break;
        case 'i':
                if (pCtx->state == SDP_SESSION)
                        pCtx->msg->SetInfo(pszLine);
                else
                        pCtx->media->SetInfo(pszLine);
                break;
        case 'u':
                pCtx->msg->SetUrl(pszLine);
                break;
        case 'e':
                pCtx->msg->AddMail(pszLine);
                break;
        case 'p':
                pCtx->msg->AddPhone(pszLine);
                break;
        case 'c':
                {
                        SDPConnection *pConn;
                        char *str2;

                        pConn = new SDPConnection;
                        if (pConn)
                        {
                                str2 = p;
                                while ((str2 = strchr (str2, '/')))
                                        *str2++ = ' ';
                                
                                read_string(str, sizeof(str), &p);
                                pConn->SetNetType(str);

                                read_string(str, sizeof(str), &p);
                                pConn->SetAddrType(str);
                                
                                read_string(str, sizeof(str), &p);
                                pConn->SetAddress(str);
                                
                                /* only read TTL for IP4 */
                                if (strcmp (pConn->GetAddrType(), "IP4") == 0)
                                {
                                        read_string(str, sizeof(str), &p);
                                        pConn->SetAddrNum(strtoul (str, NULL, 10));
                                }

                                if (pCtx->state == SDP_SESSION)
                                        m_pConnection = pConn;
                                else
                                        pCtx->media->AddConnection(pConn);
                        }
                        break;
                }
        case 'b':
                {
                        char str2[1024];

                        read_string_del (str, sizeof (str), ':', &p);
                        if (*p != '\0')
                        p++;
                        read_string (str2, sizeof (str2), &p);
                        if (pCtx->state == SDP_SESSION)
                                pCtx->msg->AddBandwidth(str, str2);
                        else
                                pCtx->media->AddBandwidth(str, str2);
                        break;
                }
        case 't':
                break;
        case 'k':
                break;
        case 'a':
                read_string_del (str, sizeof (str), ':', &p);
                if (*p != '\0')
                        p++;
                if (pCtx->state == SDP_SESSION)
                        pCtx->msg->AddAttribute(str, p);
                else
                        pCtx->media->AddAttribute(str, p);
                break;
        case 'm':
                {
                        char *slash;
                        SDPMedia *pMedia;

                        pCtx->state = SDP_MEDIA;

                        pMedia = new SDPMedia;
                        if (pMedia)
                        {
                                /* m=<media> <port>/<number of ports> <proto> <fmt> ... */
                                read_string(str, sizeof(str), &p);
                                pMedia->SetMedia(str);
                        
                                read_string (str, sizeof (str), &p);
                                slash = strrchr (str, '/');
                                if (slash)
                                {
                                        *slash = '\0';
                                        pMedia->SetSDPort(atoi(str));
                                        pMedia->SetPortNum(atoi(slash + 1));
                                } 
                                else
                                {
                                        pMedia->SetSDPort(atoi(str));
                                        pMedia->SetPortNum(0xFFFFFFFF);
                                }

                                read_string(str, sizeof(str), &p);
                                pMedia->SetProtocol(str);

                                do
                                {
                                        read_string (str, sizeof (str), &p);
                                        pMedia->AddFormat(str);
                                } while (*p != '\0');

                                pCtx->msg->AddMedia(pMedia);
                                pCtx->media = pMedia;
                        }
                        break;
                }
        default:
                break;
        }

        return 0;
}