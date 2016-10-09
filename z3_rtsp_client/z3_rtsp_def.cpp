#include "z3_common.hpp"
#include "z3_rtsp_def.hpp"
#include "z3_rtsp_error.hpp"

using namespace Z3;

#define MAX_OPTIOINS_BUF		128

static const char *RTSP_METHODS[] = {
        "DESCRIBE",
        "ANNOUNCE",
        "GET_PARAMETER",
        "OPTIONS",
        "PAUSE",
        "PLAY",
        "RECORD",
        "REDIRECT",
        "SETUP",
        "SET_PARAMETER",
        "TEARDOWN",
        "GET",
        "POST",
        NULL
};

static struct RTSP_HEADER RTSP_HEADERS[] = {
        {"Accept", 1},
        {"Accept-Encoding", 1},
        {"Accept-Language", 1},
        {"Allow", 1},
        {"Authorization", 0},
        {"Bandwidth", 0},
        {"Blocksize", 0},
        {"Cache-Control", 1},
        {"Conference", 0},
        {"Connection", 1},
        {"Content-Base", 0},
        {"Content-Encoding", 1},
        {"Content-Language", 1},
        {"Content-Length", 0},
        {"Content-Location", 0},
        {"Content-Type", 0},
        {"CSeq", 0},
        {"Date", 0},
        {"Expires", 0},
        {"From", 0},
        {"If-Modified-Since", 0},
        {"Last-Modified", 0},
        {"Proxy-Authenticate", 1},
        {"Proxy-Require", 1},
        {"Public", 1},
        {"Range", 0},
        {"Referer", 0},
        {"Require", 1},
        {"Retry-After", 0},
        {"RTP-Info", 1},
        {"Scale", 0},
        {"Session", 0},
        {"Server", 0},
        {"Speed", 0},
        {"Transport", 1},
        {"Unsupported", 0},
        {"User-Agent", 0},
        {"Via", 1},
        {"WWW-Authenticate", 1},

        /* Since 0.10.16 */
        {"Location", 0},

        /* Since 0.10.23 */
        {"ETag", 0},
        {"If-Match", 1},

        /* Since 0.10.24 */
        {"Timestamp", 0},

        /* Since 0.10.25 */
        {"Authentication-Info", 0},
        {"Host", 0},
        {"Pragma", 1},
        {"X-Server-IP-Address", 0},
        {"X-Sessioncookie", 0},

        {NULL, 0}
};


const char * Z3::RTSP_OPTIONS_AS_TEXT(RTSP_METHOD options)
{
	char *str, *end;

	str = (char *)z3_malloc(MAX_OPTIOINS_BUF);
	str[0] = '\0';

	if (options & RTSP_OPTIONS)
		strcat(str, "OPTIONS, ");
	if (options & RTSP_DESCRIBE)
		strcat(str, "DESCRIBE, ");
	if (options & RTSP_ANNOUNCE)
	    strcat(str, "ANNOUNCE, ");
	if (options & RTSP_PAUSE)
		strcat(str, "PAUSE, ");
	if (options & RTSP_PLAY)
		strcat(str, "PLAY, ");
	if (options & RTSP_RECORD)
		strcat(str, "RECORD, ");
	if (options & RTSP_REDIRECT)
		strcat(str, "REDIRECT, ");
	if (options & RTSP_SETUP)
		strcat(str, "SETUP, ");
	if (options & RTSP_GET_PARAMETER)
		strcat(str, "GET_PARAMETER, ");
	if (options & RTSP_SET_PARAMETER)
		strcat(str, "SET_PARAMETER, ");
	if (options & RTSP_TEARDOWN)
		strcat(str, "TEARDOWN, ");

	end = str + strlen(str) - 1;
	while (end > str)
	{
		if (*end == ',')
		{
			*end = 0;
			break;
		}
		*end = 0;
		--end;
	}

	return str;
}

const char* Z3::RTSP_HEADER_AS_TEXT(RTSP_HEADER_FIELD field)
{
	if (field == RTSP_HDR_INVALID)
		return NULL;
	else
		return RTSP_HEADERS[field - 1].name;
}

const char* Z3::RTSP_METHOD_AS_TEXT(RTSP_METHOD method)
{
	int i;

	if (method == RTSP_INVALID)
		return NULL;

	i = 0;
	while ((method & 1) == 0)
	{
		i++;
		method >>= 1;
	}

	return RTSP_METHODS[i];
}

RTSP_METHOD Z3::RtspFindMethod (const char * method)
{
        int idx;

        for (idx = 0; RTSP_METHODS[idx]; idx++)
        {
                if (::_stricmp(RTSP_METHODS[idx], method) == 0)
                {
                        return (RTSP_METHOD)(1 << idx);
                }
        }

        return RTSP_INVALID;
}

void Z3::TrimRightSpace(char *pLine)
{
        int nIndex;
        int nLength = strlen(pLine);

        for(nIndex = nLength - 1; nIndex >= 0; nIndex--)
        {
                if (pLine[nIndex] == ' ')
                        pLine[nIndex] = '\0';
                else
                        break;
        }

        return;
}

RTSP_HEADER_FIELD Z3::RtspFindHeaderField(const char * pHeader)
{
        int idx;

        for (idx = 0; RTSP_HEADERS[idx].name; idx++)
        {
                if (_stricmp (RTSP_HEADERS[idx].name, pHeader) == 0) 
                {
                        return (RTSP_HEADER_FIELD)(idx + 1);
                }
        }

        return RTSP_HDR_INVALID;
}

bool Z3::RtspHeaderAllowMultiple(RTSP_HEADER_FIELD field)
{
        if (field == RTSP_HDR_INVALID)
                return 0;
        else
                return RTSP_HEADERS[field - 1].multiple;
}

/* convert all consecutive whitespace to a single space */
unsigned int Z3::NormalizeLine(unsigned char *pBuffer)
{
        unsigned int nCounter = 0;

        while (*pBuffer)
        {
                if (isspace (*pBuffer))
                {
                        unsigned char *tmp;

                        *pBuffer++ = ' ';
                        for (tmp = pBuffer; isspace (*tmp); tmp++)
                        {
                                nCounter++;
                        }

                        if (pBuffer != tmp)
                                memmove (pBuffer, tmp, strlen ((char *) tmp) + 1);
                } 
                else
                {
                        pBuffer++;
                }
        }

        return nCounter;
}

int Z3::ParseString(char * pDest, int nSize, char ** pSrc)
{
        int res = Z3_EOK;
        int idx;

        idx = 0;

        /* skip spaces */
        while (isspace (**pSrc))
                (*pSrc)++;

        while (!isspace (**pSrc) && **pSrc != '\0')
        {
                if (idx < nSize - 1)
                        pDest[idx++] = **pSrc;
                else
                {
                        res = Z3_RTSP_EPARSE;
                        break;
                }

                (*pSrc)++;
        }

        if (nSize > 0)
                pDest[idx] = '\0';

        return res;
}

int Z3::ParseProtocolVersion(char *pProtocol, RTSP_VERSION *pVersion)
{
	int res = Z3_EOK;
	char *ver;

	if ((ver = strchr (pProtocol, '/')) != NULL)
        {
		unsigned int major;
		unsigned int minor;
		char dummychar;

		*ver++ = '\0';

		/* the version number must be formatted as X.Y with nothing following */
		if (sscanf_s(ver, "%u.%u%c", &major, &minor, &dummychar) != 2)
			res = Z3_RTSP_EPARSE;

		if (_stricmp(pProtocol, "RTSP") == 0)
                {
			if (major != 1 || minor != 0)
                        {
				*pVersion = RTSP_VERSION_INVALID;
				res = Z3_ERROR;
                        }
		} 
                else
			res = Z3_RTSP_EPARSE;
	} 
        else
		res = Z3_RTSP_EPARSE;

	return res;
}

/*
 *	把字符串最后的所有空格字符去掉
 *	空格字符包括 空格('')、定位字符('\t')、CR('\r')、换行('\n')、
 *	垂直定位字符('\v')或翻页('\f')的情况
 */
char * Z3::strchomp(char *str)
{
	size_t len;

	if (str == NULL)
                return NULL;

	len = strlen(str);
	while (len--)
	{
		if (isspace(str[len]))
			str[len] = '\0';
		else
			break;
	}

	return str;
}

char* Z3::z3_strdup(const char *str)
{
	char *new_str;
	size_t length, size;

	if (str)
	{
		length = strlen(str);
                size = length + 1;
		new_str = (char *)z3_calloc(size, sizeof(char));
		strncpy_s(new_str, size, str, length);
	}
	else
		new_str = NULL;

	return new_str;
}

void Z3::read_string(char * dest, unsigned int size, char ** src)
{
        unsigned int idx;

        idx = 0;

        /* skip spaces */
        while (isspace (**src))
                (*src)++;

        while (!isspace (**src) && **src != '\0')
        {
                if (idx < size - 1)
                        dest[idx++] = **src;
                (*src)++;
        }

        if (size > 0)
                dest[idx] = '\0';
}

void Z3::read_string_del (char * dest, unsigned int size, char del, char ** src)
{
        unsigned int idx;

        idx = 0;
        /* skip spaces */
        while (isspace (**src))
                (*src)++;

        while (**src != del && **src != '\0')
        {
                if (idx < size - 1)
                        dest[idx++] = **src;
                (*src)++;
        }

        if (size > 0)
                dest[idx] = '\0';
}

void Z3::GenerateDateString(char *pDateString, unsigned int nLen)
{
        static const char wkdays[7][4] = { "Sun", "Mon", "Tue", 
                                        "Wed", "Thu", "Fri", "Sat" };
        static const char months[12][4] = 
                                { "Jan", "Feb", "Mar", "Apr", "May",
                                "Jun", "Jul", "Aug", "Sep", "Oct",
                                "Nov", "Dec"
                                };
        struct tm tm;
        time_t t;

        time(&t);
        gmtime_s(&tm, &t);

        _snprintf_s(pDateString, nLen, nLen, "%s, %02d %s %04d %02d:%02d:%02d GMT",
                wkdays[tm.tm_wday], tm.tm_mday, months[tm.tm_mon], tm.tm_year + 1900,
                tm.tm_hour, tm.tm_min, tm.tm_sec);
}

const char* Z3::RTSP_STATUS_AS_TEXT(RTSP_STATUS_CODE code)
{
	switch (code)
	{
	case RTSP_STS_CONTINUE:
		return ("Continue");
	case RTSP_STS_OK:
		return ("OK");
	case RTSP_STS_CREATED:
		return ("Created");
	case RTSP_STS_LOW_ON_STORAGE:
		return ("Low on Storage Space");
	case RTSP_STS_MULTIPLE_CHOICES:
		return ("Multiple Choices");
	case RTSP_STS_MOVED_PERMANENTLY:
		return ("Moved Permanently");
	case RTSP_STS_MOVE_TEMPORARILY:
		return ("Move Temporarily");
	case RTSP_STS_SEE_OTHER:
		return ("See Other");
	case RTSP_STS_NOT_MODIFIED:
		return ("Not Modified");
	case RTSP_STS_USE_PROXY:
		return ("Use Proxy");
	case RTSP_STS_BAD_REQUEST:
		return ("Bad Request");
	case RTSP_STS_UNAUTHORIZED:
		return ("Unauthorized");
	case RTSP_STS_PAYMENT_REQUIRED:
		return ("Payment Required");
	case RTSP_STS_FORBIDDEN:
		return ("Forbidden");
	case RTSP_STS_NOT_FOUND:
		return ("Not Found");
	case RTSP_STS_METHOD_NOT_ALLOWED:
		return ("Method Not Allowed");
	case RTSP_STS_NOT_ACCEPTABLE:
		return ("Not Acceptable");
	case RTSP_STS_PROXY_AUTH_REQUIRED:
		return ( "Proxy Authentication Required");
	case RTSP_STS_REQUEST_TIMEOUT:
		return ("Request Time-out");
	case RTSP_STS_GONE:
		return ("Gone");
	case RTSP_STS_LENGTH_REQUIRED:
		return ("Length Required");
	case RTSP_STS_PRECONDITION_FAILED:
		return ("Precondition Failed");
	case RTSP_STS_REQUEST_ENTITY_TOO_LARGE:
		return ( "Request Entity Too Large");
	case RTSP_STS_REQUEST_URI_TOO_LARGE:
		return ("Request-URI Too Large");
	case RTSP_STS_UNSUPPORTED_MEDIA_TYPE:
		return ("Unsupported Media Type");
	case RTSP_STS_PARAMETER_NOT_UNDERSTOOD:
		return ( "Parameter Not Understood");
	case RTSP_STS_CONFERENCE_NOT_FOUND:
		return ("Conference Not Found");
	case RTSP_STS_NOT_ENOUGH_BANDWIDTH:
		return ("Not Enough Bandwidth");
	case RTSP_STS_SESSION_NOT_FOUND:
		return ("Session Not Found");
	case RTSP_STS_METHOD_NOT_VALID_IN_THIS_STATE:
		return ( "Method Not Valid in This State");
	case RTSP_STS_HEADER_FIELD_NOT_VALID_FOR_RESOURCE:
		return ( "Header Field Not Valid for Resource");
	case RTSP_STS_INVALID_RANGE:
		return ("Invalid Range");
	case RTSP_STS_PARAMETER_IS_READONLY:
		return ("Parameter Is Read-Only");
	case RTSP_STS_AGGREGATE_OPERATION_NOT_ALLOWED:
		return ( "Aggregate operation not allowed");
	case RTSP_STS_ONLY_AGGREGATE_OPERATION_ALLOWED:
		return ( "Only aggregate operation allowed");
	case RTSP_STS_UNSUPPORTED_TRANSPORT:
		return ("Unsupported transport");
	case RTSP_STS_DESTINATION_UNREACHABLE:
		return ("Destination unreachable");
	case RTSP_STS_INTERNAL_SERVER_ERROR:
		return ("Internal Server Error");
	case RTSP_STS_NOT_IMPLEMENTED:
		return ("Not Implemented");
	case RTSP_STS_BAD_GATEWAY:
		return ("Bad Gateway");
	case RTSP_STS_SERVICE_UNAVAILABLE:
		return ("Service Unavailable");
	case RTSP_STS_GATEWAY_TIMEOUT:
		return ("Gateway Time-out");
	case RTSP_STS_RTSP_VERSION_NOT_SUPPORTED:
		return ( "RTSP Version not supported");
	case RTSP_STS_OPTION_NOT_SUPPORTED:
		return ("Option not supported");
	default:
		return ("Unknown Method");
	}
}


RTSP_STATUS_CODE Z3::RTSP_TRANS_STATUS_CODE(int err)
{
    RTSP_STATUS_CODE rsc;

    switch (err)
    {
    case -EINVAL:
        rsc = RTSP_STS_BAD_REQUEST;
        break;

    case -ENOMEM:
        rsc = RTSP_STS_INTERNAL_SERVER_ERROR;
        break;

    default:
        rsc = RTSP_STS_BAD_REQUEST;
        break;
    }

    return rsc;
}