#ifndef _Z3_RTSP_DEF_HPP_
#define _Z3_RTSP_DEF_HPP_

#define DEFAULT_RTSP_PORT       8554
#define RTSP_RECVBUF_SIZE       4096
#define H264_MAX_FRAME_SIZE     2097152
#define UDP_RECV_TIMEOUT        3000
#define DEFAULT_RTSP_SESSION_TIMEOUT    15000

namespace Z3 {

        enum _PROTO_ID
        {
                UNKNOWN_PROTO = 0,
                RTSP_PROTO_ID,
                RTP_PROTO_ID,
                RTCP_PROTO_ID,
                MEDIA_FRAME_ID
        };

        struct RTSP_HEADER
        {
	        char    *name;
	        bool    multiple;	//true or false
        };

        /**
         * @RTSP_OK: no error
         * @RTSP_ERROR: some unspecified error occured
         * @RTSP_EINVAL: invalid arguments were provided to a function
         * @RTSP_EINTR: an operation was canceled
         * @RTSP_ENOMEM: no memory was available for the operation
         * @RTSP_ERESOLV: a host resolve error occured
         * @RTSP_ENOTIMPL: function not implemented
         * @RTSP_ESYS: a system error occured, errno contains more details
         * @RTSP_EPARSE: a persing error occured
         * @RTSP_EWSASTART: windows networking could not start
         * @RTSP_EWSAVERSION: windows networking stack has wrong version
         * @RTSP_EEOF: end-of-file was reached
         * @RTSP_ENET: a network problem occured, h_errno contains more details
         * @RTSP_ENOTIP: the host is not an IP host
         * @RTSP_ETIMEOUT: a timeout occured
         * @RTSP_ETGET: the tunnel GET request has been performed
         * @RTSP_ETPOST: the tunnel POST request has been performed
         * @RTSP_ELAST: last error
         *
         * Result codes from the RTSP functions.
         */
        //enum _RTSP_RESULT {
        //        RTSP_OK          =  0,
        //        /* errors */
        //        RTSP_ERROR       = -1,
        //        RTSP_EINVAL      = -2,
        //        RTSP_EINTR       = -3,
        //        RTSP_ENOMEM      = -4,
        //        RTSP_ERESOLV     = -5,
        //        RTSP_ENOTIMPL    = -6,
        //        RTSP_ESYS        = -7,
        //        RTSP_EPARSE      = -8,
        //        RTSP_EWSASTART   = -9,
        //        RTSP_EWSAVERSION = -10,
        //        RTSP_EEOF        = -11,
        //        RTSP_ENET        = -12,
        //        RTSP_ENOTIP      = -13,
        //        RTSP_ETIMEOUT    = -14,
        //        RTSP_ETGET       = -15,
        //        RTSP_ETPOST      = -16,

        //        RTSP_ELAST       = -17
        //};
        //typedef int     int;

        /**
         * @RTSP_VERSION_INVALID: unknown/invalid version
         * @RTSP_VERSION_1_0: version 1.0
         * @RTSP_VERSION_1_1: version 1.1. Since 0.10.25
         *
         * The supported RTSP versions.
         */
        enum _RTSP_VERSION {
                RTSP_VERSION_INVALID = 0x00,
                RTSP_VERSION_1_0     = 0x10,
                RTSP_VERSION_1_1     = 0x11
        };
        typedef unsigned int    RTSP_VERSION;

        /**
         * @RTSP_INVALID: invalid method
         * @RTSP_DESCRIBE: the DESCRIBE method
         * @RTSP_ANNOUNCE: the ANNOUNCE method
         * @RTSP_GET_PARAMETER: the GET_PARAMETER method
         * @RTSP_OPTIONS: the OPTIONS method
         * @RTSP_PAUSE: the PAUSE method
         * @RTSP_PLAY: the PLAY method
         * @RTSP_RECORD: the RECORD method
         * @RTSP_REDIRECT: the REDIRECT method
         * @RTSP_SETUP: the SETUP method
         * @RTSP_SET_PARAMETER: the SET_PARAMETER method
         * @RTSP_TEARDOWN: the TEARDOWN method
         * @RTSP_GET: the GET method (HTTP). Since 0.10.25
         * @RTSP_POST: the POST method (HTTP). Since 0.10.25
         *
         * The different supported RTSP methods. 
         */
        enum _RTSP_METHOD {
                RTSP_INVALID          = 0,
                RTSP_DESCRIBE         = (1 <<  0),
                RTSP_ANNOUNCE         = (1 <<  1),
                RTSP_GET_PARAMETER    = (1 <<  2),
                RTSP_OPTIONS          = (1 <<  3),
                RTSP_PAUSE            = (1 <<  4),
                RTSP_PLAY             = (1 <<  5),
                RTSP_RECORD           = (1 <<  6),
                RTSP_REDIRECT         = (1 <<  7),
                RTSP_SETUP            = (1 <<  8),
                RTSP_SET_PARAMETER    = (1 <<  9),
                RTSP_TEARDOWN         = (1 << 10),
                RTSP_GET              = (1 << 11),
                RTSP_POST             = (1 << 12)
        };
        typedef unsigned int    RTSP_METHOD;

        enum _RTSP_HEADER_FIELD {
                RTSP_HDR_INVALID,

                /*
                * R = Request
                * r = response
                * g = general
                * e = entity
                */
                RTSP_HDR_ACCEPT,              /* Accept               R      opt.      entity */
                RTSP_HDR_ACCEPT_ENCODING,     /* Accept-Encoding      R      opt.      entity */
                RTSP_HDR_ACCEPT_LANGUAGE,     /* Accept-Language      R      opt.      all */
                RTSP_HDR_ALLOW,               /* Allow                r      opt.      all */
                RTSP_HDR_AUTHORIZATION,       /* Authorization        R      opt.      all */
                RTSP_HDR_BANDWIDTH,           /* Bandwidth            R      opt.      all */
                RTSP_HDR_BLOCKSIZE,           /* Blocksize            R      opt.      all but OPTIONS, TEARDOWN */
                RTSP_HDR_CACHE_CONTROL,       /* Cache-Control        g      opt.      SETUP */
                RTSP_HDR_CONFERENCE,          /* Conference           R      opt.      SETUP */
                RTSP_HDR_CONNECTION,          /* Connection           g      req.      all */
                RTSP_HDR_CONTENT_BASE,        /* Content-Base         e      opt.      entity */
                RTSP_HDR_CONTENT_ENCODING,    /* Content-Encoding     e      req.      SET_PARAMETER, DESCRIBE, ANNOUNCE */
                RTSP_HDR_CONTENT_LANGUAGE,    /* Content-Language     e      req.      DESCRIBE, ANNOUNCE */
                RTSP_HDR_CONTENT_LENGTH,      /* Content-Length       e      req.      SET_PARAMETER, ANNOUNCE, entity */
                RTSP_HDR_CONTENT_LOCATION,    /* Content-Location     e      opt.      entity */
                RTSP_HDR_CONTENT_TYPE,        /* Content-Type         e      req.      SET_PARAMETER, ANNOUNCE, entity */
                RTSP_HDR_CSEQ,                /* CSeq                 g      req.      all */
                RTSP_HDR_DATE,                /* Date                 g      opt.      all */
                RTSP_HDR_EXPIRES,             /* Expires              e      opt.      DESCRIBE, ANNOUNCE */
                RTSP_HDR_FROM,                /* From                 R      opt.      all */
                RTSP_HDR_IF_MODIFIED_SINCE,   /* If-Modified-Since    R      opt.      DESCRIBE, SETUP */
                RTSP_HDR_LAST_MODIFIED,       /* Last-Modified        e      opt.      entity */
                RTSP_HDR_PROXY_AUTHENTICATE,  /* Proxy-Authenticate */
                RTSP_HDR_PROXY_REQUIRE,       /* Proxy-Require        R      req.      all */
                RTSP_HDR_PUBLIC,              /* Public               r      opt.      all */
                RTSP_HDR_RANGE,               /* Range                Rr     opt.      PLAY, PAUSE, RECORD */
                RTSP_HDR_REFERER,             /* Referer              R      opt.      all */
                RTSP_HDR_REQUIRE,             /* Require              R      req.      all */
                RTSP_HDR_RETRY_AFTER,         /* Retry-After          r      opt.      all */
                RTSP_HDR_RTP_INFO,            /* RTP-Info             r      req.      PLAY */
                RTSP_HDR_SCALE,               /* Scale                Rr     opt.      PLAY, RECORD */
                RTSP_HDR_SESSION,             /* Session              Rr     req.      all but SETUP, OPTIONS */
                RTSP_HDR_SERVER,              /* Server               r      opt.      all */
                RTSP_HDR_SPEED,               /* Speed                Rr     opt.      PLAY */
                RTSP_HDR_TRANSPORT,           /* Transport            Rr     req.      SETUP */
                RTSP_HDR_UNSUPPORTED,         /* Unsupported          r      req.      all */
                RTSP_HDR_USER_AGENT,          /* User-Agent           R      opt.      all */
                RTSP_HDR_VIA,                 /* Via                  g      opt.      all */
                RTSP_HDR_WWW_AUTHENTICATE,    /* WWW-Authenticate     r      opt.      all */

                /* Since 0.10.16 */
                RTSP_HDR_LOCATION,            /* Location */

                /* Since 0.10.23 */
                RTSP_HDR_ETAG,                /* ETag */
                RTSP_HDR_IF_MATCH,            /* If-Match */

                /* Since 0.10.24 */
                RTSP_HDR_TIMESTAMP,           /* Timestamp */

                /* Since 0.10.25 */
                RTSP_HDR_AUTHENTICATION_INFO, /* Authentication-Info */
                RTSP_HDR_HOST,                /* Host */
                RTSP_HDR_PRAGMA,              /* Pragma */
                RTSP_HDR_X_SERVER_IP_ADDRESS, /* X-Server-IP-Address */
                RTSP_HDR_X_SESSIONCOOKIE,     /* X-Sessioncookie */

                RTSP_HDR_LAST
        };
        typedef unsigned int    RTSP_HEADER_FIELD;

        enum _RTSP_STATUS_CODE {
                RTSP_STS_INVALID                              = 0, 
                RTSP_STS_CONTINUE                             = 100, 
                RTSP_STS_OK                                   = 200, 
                RTSP_STS_CREATED                              = 201, 
                RTSP_STS_LOW_ON_STORAGE                       = 250, 
                RTSP_STS_MULTIPLE_CHOICES                     = 300, 
                RTSP_STS_MOVED_PERMANENTLY                    = 301, 
                RTSP_STS_MOVE_TEMPORARILY                     = 302, 
                RTSP_STS_SEE_OTHER                            = 303, 
                RTSP_STS_NOT_MODIFIED                         = 304, 
                RTSP_STS_USE_PROXY                            = 305, 
                RTSP_STS_BAD_REQUEST                          = 400, 
                RTSP_STS_UNAUTHORIZED                         = 401, 
                RTSP_STS_PAYMENT_REQUIRED                     = 402, 
                RTSP_STS_FORBIDDEN                            = 403, 
                RTSP_STS_NOT_FOUND                            = 404, 
                RTSP_STS_METHOD_NOT_ALLOWED                   = 405, 
                RTSP_STS_NOT_ACCEPTABLE                       = 406, 
                RTSP_STS_PROXY_AUTH_REQUIRED                  = 407, 
                RTSP_STS_REQUEST_TIMEOUT                      = 408, 
                RTSP_STS_GONE                                 = 410, 
                RTSP_STS_LENGTH_REQUIRED                      = 411, 
                RTSP_STS_PRECONDITION_FAILED                  = 412, 
                RTSP_STS_REQUEST_ENTITY_TOO_LARGE             = 413, 
                RTSP_STS_REQUEST_URI_TOO_LARGE                = 414, 
                RTSP_STS_UNSUPPORTED_MEDIA_TYPE               = 415, 
                RTSP_STS_PARAMETER_NOT_UNDERSTOOD             = 451, 
                RTSP_STS_CONFERENCE_NOT_FOUND                 = 452, 
                RTSP_STS_NOT_ENOUGH_BANDWIDTH                 = 453, 
                RTSP_STS_SESSION_NOT_FOUND                    = 454, 
                RTSP_STS_METHOD_NOT_VALID_IN_THIS_STATE       = 455, 
                RTSP_STS_HEADER_FIELD_NOT_VALID_FOR_RESOURCE  = 456, 
                RTSP_STS_INVALID_RANGE                        = 457, 
                RTSP_STS_PARAMETER_IS_READONLY                = 458, 
                RTSP_STS_AGGREGATE_OPERATION_NOT_ALLOWED      = 459, 
                RTSP_STS_ONLY_AGGREGATE_OPERATION_ALLOWED     = 460, 
                RTSP_STS_UNSUPPORTED_TRANSPORT                = 461, 
                RTSP_STS_DESTINATION_UNREACHABLE              = 462, 
                RTSP_STS_INTERNAL_SERVER_ERROR                = 500, 
                RTSP_STS_NOT_IMPLEMENTED                      = 501, 
                RTSP_STS_BAD_GATEWAY                          = 502, 
                RTSP_STS_SERVICE_UNAVAILABLE                  = 503, 
                RTSP_STS_GATEWAY_TIMEOUT                      = 504, 
                RTSP_STS_RTSP_VERSION_NOT_SUPPORTED           = 505, 
                RTSP_STS_OPTION_NOT_SUPPORTED                 = 551
        };
        typedef unsigned int    RTSP_STATUS_CODE;

        const char*             RTSP_OPTIONS_AS_TEXT(RTSP_METHOD options);
        const char*             RTSP_HEADER_AS_TEXT(RTSP_HEADER_FIELD field);
        const char*             RTSP_METHOD_AS_TEXT(RTSP_METHOD method);
        const char*             RTSP_STATUS_AS_TEXT(RTSP_STATUS_CODE code);
        RTSP_STATUS_CODE        RTSP_TRANS_STATUS_CODE(int err);

        RTSP_METHOD             RtspFindMethod (const char * method);
        RTSP_HEADER_FIELD       RtspFindHeaderField(const char * pHeader);
        bool                    RtspHeaderAllowMultiple(RTSP_HEADER_FIELD field);

        /* convert all consecutive whitespace to a single space */
        unsigned int            NormalizeLine (unsigned char *pBuffer);
        void                    TrimRightSpace(char *pLine);
        void                    GenerateDateString(char *pDateString, unsigned int nLen);

        /*
         *	把字符串最后的所有空格字符去掉
         *	空格字符包括 空格('')、定位字符('\t')、CR('\r')、换行('\n')、
         *	垂直定位字符('\v')或翻页('\f')的情况
         */
        char *                  strchomp(char *str);
        
        char*                   z3_strdup(const char *str);

        void                    read_string(char * dest, unsigned int size, char ** src);
        void                    read_string_del (char * dest, unsigned int size, char del, char ** src);

        int     ParseString(char* pDest, int nSize, char ** pSrc);
        int     ParseProtocolVersion(char *pProtocol, RTSP_VERSION *pVersion);


};

#endif