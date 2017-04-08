#ifndef _Z3_SDP_HPP_
#define _Z3_SDP_HPP_

#include <vector>
#include <string>
#include "z3_async_obj.hpp"

namespace Z3 {

        typedef std::string STL_STRING;
        typedef std::vector<STL_STRING *> STL_STRING_VECTOR;

        /**
         * SDPBandwidth:
         * @m_pszBWType: the bandwidth modifier type
         * @m_nBandwidth: the bandwidth in kilobits per second
         *
         * The contents of the SDP "b=" field which specifies the proposed bandwidth to
         * be used by the session or media.
         */
        class SDPBandwidth : public MemoryObject
        {
        public:
                SDPBandwidth();
                ~SDPBandwidth();

                bool    SetBandwidth(char *pszType, unsigned int nBandwidth);

        private:
                char            *m_pszBWType;
                unsigned int    m_nBandwidth;
        };

        /**
         * SDPTime:
         * @m_pszStart: start time for the conference. The value is the decimal
         *     representation of Network Time Protocol (NTP) time values in seconds
         * @m_pszStop: stop time for the conference. The value is the decimal
         *     representation of Network Time Protocol (NTP) time values in seconds
         * @m_pRepeat: repeat times for a session
         *
         * The contents of the SDP "t=" field which specify the start and stop times for
         * a conference session.
         */
        class SDPTime : public MemoryObject
        {
        public:
                SDPTime();
                ~SDPTime();

        private:
                char            *m_pszStart;
                char            *m_pszStop;
                STL_STRING_VECTOR       *m_pRepeat;
        };


        /**
         * SDPZone:
         * @m_pszTime: the NTP time that a time zone adjustment happens
         * @m_pszTypedTime: the offset from the time when the session was first scheduled
         *
         * The contents of the SDP "z=" field which allows the sender to
         * specify a list of time zone adjustments and offsets from the base
         * time.
         */
        class SDPZone : public MemoryObject
        {
        public:
                SDPZone();
                ~SDPZone();

        private:
                char    *m_pszTime;
                char    *m_pszTypedTime;
        };

        /**
         * SDPOrigin:
         * @m_pszUsername: the user's login on the originating host, or it is "-"
         *    if the originating host does not support the concept of user ids.
         * @m_pszSessId: is a numeric string such that the tuple of @m_pszUsername, 
         *    @m_pszSessId, @m_pszNettype, @m_pszAddrType and @m_pszAddr form a 
         *    globally unique identifier for the session.
         * @m_pszSessVersion: a version number for this announcement
         * @m_pszNettype: the type of network. "IN" is defined to have the meaning
         *    "Internet".
         * @m_pszAddrType: the type of @m_pszAddr.
         * @m_pszAddr: the globally unique address of the machine from which the session was
         *     created.
         *
         * The contents of the SDP "o=" field which gives the originator of the session
         * (their username and the address of the user's host) plus a session id and
         * session version number.
         */
        class SDPOrigin : public MemoryObject
        {
        public:
                SDPOrigin();
                ~SDPOrigin();

                int     SetUsername(char *pszUserName);
                char *  GetUsername();

                int     SetSessId(char *pszSessId);
                char *  GetSessId();

                int     SetSessVersion(char *pszSessVersion);
                char *  GetSessVersion();

                int     SetNetType(char *pszNetType);
                char *  GetNetType();

                int     SetAddrType(char *pszAddrType);
                char *  GetAddrType();

                int     SetAddr(char *pszAddr);
                char *  GetAddr();

        private:
                int     SetField(char **ppDest, char *pSrc);

                char *m_pszUsername;
                char *m_pszSessId;
                char *m_pszSessVersion;
                char *m_pszNetType;
                char *m_pszAddrType;
                char *m_pszAddr;
        };

       /**
        * SDPConnection:
        * @m_pszNetType: the type of network. "IN" is defined to have the meaning
        *    "Internet".
        * @m_pszAddrType: the type of @address.
        * @m_pszAddress: the address
        * @m_nTTL: the time to live of the address
        * @m_nAddrNum: the number of layers
        */
        class SDPConnection : public MemoryObject
        {
        public:
                SDPConnection();
                ~SDPConnection();

                SDPConnection& operator = (SDPConnection &conn);

                int             SetNetType(char *pszNetType);
                
                int             SetAddrType(char *pszAddrType);
                const char *    GetAddrType();

                int             SetAddress(char *pszAddress);
                void            SetTTL(unsigned int nTtl);
                void            SetAddrNum(unsigned int num);

        private:
                char            *m_pszNetType;
                char            *m_pszAddrType;
                char            *m_pszAddress;
                unsigned int    m_nTTL;
                unsigned int    m_nAddrNum;
        };

        class SDPKey : public MemoryObject
        {
        public:
                SDPKey();
                ~SDPKey();

        private:
                char    *m_pszType;
                char    *m_pszData;
        };

        class SDPAttribute : public MemoryObject
        {
        public:
                SDPAttribute();
                ~SDPAttribute();

                bool    SetAttribute(char *pszKey, char *pszValue);
                inline char *   GetKey() { return m_pszKey; }
                inline char *   GetValue() { return m_pszValue; }

        private:
                char    *m_pszKey;
                char    *m_pszValue;
        };

        class SDPMedia : public MemoryObject
        {
        public:
                SDPMedia();
                ~SDPMedia();

                bool            SetMedia(char *pszMedia);
                const char *    GetMedia();

                void            SetSDPort(unsigned int nPort);
                void            SetPortNum(unsigned int nPortNum);
                bool            SetProtocol(char *pszProtocol);
                bool            AddFormat(char *pszFormat);
                int             SetInfo(char *pszInfo);
                bool            AddConnection(SDPConnection *pConnection);
                bool            AddBandwidth(char *pszType, char *pszBandwidth);
                bool            AddAttribute(char *pszKey, char *pszValue);
                char *          GetAttributeValue(char *pszKey, unsigned int nth = 0);

        private:
                char            *m_pszMedia;
                unsigned int    m_nPort;
                unsigned int    m_nPortNum;
                char            *m_pszProto;
                STL_STRING_VECTOR       *m_pVectorFmts;
                char            *m_pszInfo;
                std::vector<SDPConnection *>    *m_pVectorConnections;
                std::vector<SDPBandwidth *>     *m_pVectorBandwidths;
                SDPKey          m_key;
                std::vector<SDPAttribute *>     *m_pVectorAttributes;
        };




        /**
         * SDPMessage:
         * @version: the protocol version
         * @origin: owner/creator and session identifier
         * @session_name: session name
         * @information: session information
         * @uri: URI of description
         * @emails: array of #gchar with email addresses
         * @phones: array of #gchar with phone numbers
         * @connection: connection information for the session
         * @bandwidths: array of #SDPBandwidth with bandwidth information
         * @times: array of #SDPTime with time descriptions
         * @zones: array of #SDPZone with time zone adjustments
         * @key: encryption key
         * @attributes: array of #SDPAttribute with session attributes
         * @medias: array of #SDPMedia with media descriptions
         *
         * The contents of the SDP message.
         */
        class SDPMessage : public MemoryObject
        {
        public:
                SDPMessage();
                ~SDPMessage();

                int             ParseString(const char *pszBuf, unsigned int nBufSize);

                int             SetVersion(char *pszVersion);
                const char *    GetVersion();

                SDPOrigin &     GetOrigin();

                int             SetSessionName(char *pszSessionName);
                const char *    GetSessionName();

                int             SetInfo(char *pszInfo);
                const char *    GetInfo();

                int             SetUrl(char *pszUrl);
                const char *    GetUrl();

                bool            AddMail(char *pszMail);               
                bool            AddPhone(char *pszPhone);
                bool            AddBandwidth(char *pszType, char *pszBandwidth);
                bool            AddAttribute(char *pszKey, char *pszValue);
                
                unsigned int    GetMediaCount();
                bool            AddMedia(SDPMedia *pMedia);
                SDPMedia *      GetSDPMedia(unsigned int nIndex);

                void            SetConnection(SDPConnection *pConn);

        protected:
                typedef struct
                {
                        unsigned int    state;
                        SDPMessage      *msg;
                        SDPMedia        *media;
                } SDPContext;

                int             ParseLine(SDPContext *pCtx, char type, char *pszLine);

                bool            AddItemIntoVector(STL_STRING_VECTOR *pVector, char *pszItem);

        private:
                char            *m_pszVersion;
                SDPOrigin       m_origin;
                char            *m_pszSessionName;
                char            *m_pszInfo;
                char            *m_pszUrl;
                STL_STRING_VECTOR      *m_pVectorEmails;
                STL_STRING_VECTOR      *m_pVectorPhones;
                SDPConnection                   *m_pConnection;
                std::vector<SDPBandwidth *>     *m_pVectorBandwidths;
                std::vector<SDPTime *>          *m_pVectorTimes;
                std::vector<SDPZone *>          *m_pVectorZones;
                SDPKey          m_key;
                std::vector<SDPAttribute *>     *m_pVectorAttributes;
                std::vector<SDPMedia *>         *m_pVectorMedias;
        };

};

#endif