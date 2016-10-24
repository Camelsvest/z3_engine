#ifndef _Z3_RTCP_HPP_
#define _Z3_RTCP_HPP_

#include <stdint.h>

#pragma pack(push, 1)

typedef enum {
        RTCP_SR   = 200,
        RTCP_RR   = 201,
        RTCP_SDES = 202,        /* Դ���� */
        RTCP_BYE  = 203,
        RTCP_APP  = 204
} RTCP_TYPE;


typedef enum {              /* Դ�������� */
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
        uint8_t count:5;          /* �������� */
        uint8_t padding:1;
        uint8_t version:2;
#elif _BYTE_ORDER == _BIG_ENDIAN
        uint8_t version:2;
        uint8_t padding:1;
        uint8_t count:5;
#else
# error "Invalid byte order!"
#endif
        uint8_t packet_type;    /* RTCP�������� */
        uint16_t length;           /* RTCP����(��ͷ��4�ֽ�Ϊ��λ)-1*/
} RTCP_COMM_HEADER;


typedef struct _rtcp_src_desc {     /* Դ���� */
        uint8_t type;
        uint8_t len;
        uint8_t data[1];
} RTCP_SRC_DESC;

typedef struct _rtcp_rr_block {     /* ���ն˱���� */
        uint32_t ssrc;             /* Դʶ�� */
        uint32_t lost_rate:8;      /* ������(���ϴ�RR/SR) */
        uint32_t lost:24;          /* ������(���ϴ�RR/SR��ȫ����ʧ�򲻷���RR) */
        uint32_t last_seq;         /* ���յ������seq(��16λ), ��16λ��չΪ���ڼ��� */
        uint32_t jitter;           /* ������� */
        uint32_t lsr;              /* ��һ��SR��ʶ��NTPʱ������м�32λ */
        uint32_t dlsr;             /* �յ���һ��SR�����ʹ�RR֮�����ʱ */
} RTCP_RR_BLOCK;


typedef struct _rtcp_sr_header {    /* ���Ͷ˱��� */
        RTCP_COMM_HEADER comm_h;
        uint32_t ssrc;             /* ��SR������Դ��ʶ */
        uint32_t ntp_hi;           /* 64λNTP */
        uint32_t ntp_lo;
        uint32_t rtp_ts;           /* RTPʱ��� */
        uint32_t pkt_sent;         /* �ѷ��͵�RTP���� */
        uint32_t oct_sent;         /* �ѷ��͵�RTP�ֽ��� */
        RTCP_RR_BLOCK rr_b[1];  /* �������ն���Ϣ */
} RTCP_SR_HEADER;


typedef struct _rtcp_rr_header {
        RTCP_COMM_HEADER comm_h;
        uint32_t ssrc;             /* ��RR������Դ��ʶ */
        RTCP_RR_BLOCK rr_b[1];  /* �������ն���Ϣ */
} RTCP_RR_HEADER;


typedef struct _rtcp_sdes_header {  /* Դ���� */
        RTCP_COMM_HEADER comm_h;
        uint32_t src;
        RTCP_SRC_DESC descrip[1];
} RTCP_SDES_HEADER;


typedef struct _rtcp_bye_header {   /* �뿪ͨ�� */
        RTCP_COMM_HEADER comm_h;
        uint32_t src[1];           /* Դ��ʶ */
        uint8_t reason[1];         /* �뿪ԭ�� */
} RTCP_BYE_HEADER;


typedef struct _rtcp_app_header {
        RTCP_COMM_HEADER comm_h;
        uint32_t ssrc;             /* Դ��ʶ */
        uint8_t name[4];           /* Ӧ������ */
        uint8_t data[1];           /* Ӧ������ */
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