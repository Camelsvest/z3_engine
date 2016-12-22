#ifndef _Z3_RTP_HPP_
#define _Z3_RTP_HPP_

#if defined(WIN32) || defined(_WIN32)
#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define _BYTE_ORDER __LITTLE_ENDIAN
#else
#define _BSD_SOURCE
#include <endian.h>
#endif

typedef struct _rtp_hdr
{
#if _BYTE_ORDER == __LITTLE_ENDIAN
        unsigned short  csrc_count      :4,    /* 有贡献源个数 */
                        extension       :1, 
                        padding         :1,
                        version         :2,
                        payload_type    :7,  /* 载荷类型 */
                        marker          :1;
#elif _BYTE_ORDER == __BIG_ENDIAN
        unsigned short  version         :2,
                        padding         :1, 
                        extension       :1,
                        csrc_count      :4,
                        marker          :1,
                        payload_type    :7;
#else
        # error "Invalid byte order!"
#endif
        unsigned short  seq;
        unsigned int    timestamp;
        unsigned int    ssrc;            /* 同步源 */
        unsigned int	data[0];
} RTP_HDR;

typedef struct _rtp_ext_hdr
{
        unsigned short  reserved;
        unsigned short  length;
} RTP_EXT_HDR;

#define RTP_PKT_VER(pkt)    (((RTP_HDR *)pkt)->version)
#define RTP_PKT_EXT(pkt)    (((RTP_HDR *)pkt)->extension)
#define RTP_PKT_CC(pkt)     (((RTP_HDR *)pkt)->csrc_count)
#define RTP_PKT_MARK(pkt)   (((RTP_HDR *)pkt)->marker)
#define RTP_PKT_PT(pkt)     (((RTP_HDR *)pkt)->payload_type)
#define RTP_PKT_SEQ(pkt)    ntohs(((RTP_HDR *)pkt)->seq)
#define RTP_PKT_TS(pkt)     ntohl(((RTP_HDR *)pkt)->timestamp)
#define RTP_PKT_SSRC(pkt)   ntohl(((RTP_HDR *)pkt)->ssrc)
#define RTP_PKT_DATA(pkt)   (&((RTP_HDR *)pkt)->data[0]  + ((RTP_HDR *)pkt)->csrc_count * 4)

#define RTPPT_ISDYNAMIC(pt)     (pt >= 96)       

#endif