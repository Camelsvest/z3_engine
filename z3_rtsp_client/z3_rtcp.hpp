#ifndef _Z3_RTCP_HPP_
#define _Z3_RTCP_HPP_

#include <stdint.h>

#pragma pack(push, 1)

typedef enum {
        RTCP_SR   = 200,
        RTCP_RR   = 201,
        RTCP_SDES = 202,        /* 源描述 */
        RTCP_BYE  = 203,
        RTCP_APP  = 204
} RTCP_TYPE;


typedef enum {              /* 源描述类型 */
        RTCP_SDES_END   = 0,
        RTCP_SDES_CNAME = 1,
        RTCP_SDES_NAME  = 2,
        RTCP_SDES_EMAIL = 3,
        RTCP_SDES_PHONE = 4,
        RTCP_SDES_LOC   = 5,
        RTCP_SDES_TOOL  = 6,
        RTCP_SDES_NOTE  = 7,
        RTCP_SDES_PRIV  = 8,
        RTCP_SDES_MAX   = 9
} RTCP_SDES_TYPE;


typedef struct _rtcp_comm_header {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        uint8_t count:5;          /* 报告块计数 */
        uint8_t padding:1;
        uint8_t version:2;
#elif _BYTE_ORDER == _BIG_ENDIAN
        uint8_t version:2;
        uint8_t padding:1;
        uint8_t count:5;
#else
# error "Invalid byte order!"
#endif
        uint8_t packet_type;    /* RTCP报文类型 */
        uint16_t length;           /* RTCP包长(含头，4字节为单位)-1*/
} RTCP_COMM_HEADER;


typedef struct _rtcp_src_desc {     /* 源描述 */
        uint8_t type;
        uint8_t len;
        uint8_t data[1];
} RTCP_SRC_DESC;

typedef struct _rtcp_rr_block {     /* 接收端报告块 */
        uint32_t ssrc;             /* 源识别 */
        uint32_t lost_rate:8;      /* 丢包率(自上次RR/SR) */
        uint32_t lost:24;          /* 丢包数(自上次RR/SR，全部丢失则不发送RR) */
        uint32_t last_seq;         /* 接收到的最高seq(低16位), 高16位扩展为周期计数 */
        uint32_t jitter;           /* 间隔抖动 */
        uint32_t lsr;              /* 上一个SR标识，NTP时间戳的中间32位 */
        uint32_t dlsr;             /* 收到上一个SR到发送此RR之间的延时 */
} RTCP_RR_BLOCK;


typedef struct _rtcp_sr_header {    /* 发送端报告 */
        RTCP_COMM_HEADER comm_h;
        uint32_t ssrc;             /* 该SR发送者源标识 */
        uint32_t ntp_hi;           /* 64位NTP */
        uint32_t ntp_lo;
        uint32_t rtp_ts;           /* RTP时间戳 */
        uint32_t pkt_sent;         /* 已发送的RTP包数 */
        uint32_t oct_sent;         /* 已发送的RTP字节数 */
        RTCP_RR_BLOCK rr_b[1];  /* 反馈接收端信息 */
} RTCP_SR_HEADER;


typedef struct _rtcp_rr_header {
        RTCP_COMM_HEADER comm_h;
        uint32_t ssrc;             /* 该RR发送者源标识 */
        RTCP_RR_BLOCK rr_b[1];  /* 反馈接收端信息 */
} RTCP_RR_HEADER;


typedef struct _rtcp_sdes_header {  /* 源描述 */
        RTCP_COMM_HEADER comm_h;
        uint32_t src;
        RTCP_SRC_DESC descrip[1];
} RTCP_SDES_HEADER;


typedef struct _rtcp_bye_header {   /* 离开通告 */
        RTCP_COMM_HEADER comm_h;
        uint32_t src[1];           /* 源标识 */
        uint8_t reason[1];         /* 离开原因 */
} RTCP_BYE_HEADER;


typedef struct _rtcp_app_header {
        RTCP_COMM_HEADER comm_h;
        uint32_t ssrc;             /* 源标识 */
        uint8_t name[4];           /* 应用名称 */
        uint8_t data[1];           /* 应用数据 */
} RTCP_APP_HEADER;

#define RTCP_PKT_RC(pkt)        (((RTCP_COMM_HEADER *)pkt)->count)
#define RTCP_PKT_PT(pkt)        (((RTCP_COMM_HEADER *)pkt)->packet_type)
#define RTCP_PKT_LENGTH(pkt)    ((ntohs((((RTCP_COMM_HEADER *)pkt)->length)) + 1) * 4)
#define RTCP_PKT_PADDING(pkt)   (((RTCP_COMM_HEADER *)pkt)->padding)

#define RB_BLK_SSRC(buffer)     (((RTCP_RR_BLOCK *)buffer)->ssrc)
#define RB_BLK_LOST_RATE(buffer)        (((RTCP_RR_BLOCK *)buffer)->lost_rate)
#define RB_BLK_LOST(buffer)     (((RTCP_RR_BLOCK *)buffer)->lost)
#define RB_BLK_LAST_SEQ(buffer) (((RTCP_RR_BLOCK *)buffer)->last_seq)
#define RB_BLK_JITTER(buffer)   (((RTCP_RR_BLOCK *)buffer)->jitter)
#define RB_BLK_LSR(buffer)      (((RTCP_RR_BLOCK *)buffer)->lsr)
#define RB_BLK_DLSR(buffer)     (((RTCP_RR_BLOCK *)buffer)->dlsr)

#pragma pack(pop)

#endif