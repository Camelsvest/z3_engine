// rtp.c
#include "rtp.h"

void init_seq(source *s, u_int16 seq)
{
	s->base_seq = seq;
	s->max_seq = seq;
	s->bad_seq = RTP_SEQ_MOD + 1;   /* so seq == bad_seq is false */
	s->cycles = 0;
	s->received = 0;
	s->received_prior = 0;
	s->expected_prior = 0;
	/* other initialization */
}

int update_seq(source *s, u_int16 seq)
{
	u_int16 udelta = seq - s->max_seq;
	const int MAX_DROPOUT = 3000;
	const int MAX_MISORDER = 100;
	const int MIN_SEQUENTIAL = 2;

	/*
	* Source is not valid until MIN_SEQUENTIAL packets with
	* sequential sequence numbers have been received.
	*/
	if (s->probation) {
		/* packet is in sequence */
		if (seq == s->max_seq + 1) {
			s->probation--;
			s->max_seq = seq;
			if (s->probation == 0) {
				init_seq(s, seq);
				s->received++;
				return 1;
			}
		} else {
			s->probation = MIN_SEQUENTIAL - 1;
			s->max_seq = seq;
		}
		return 0;
	} else if (udelta < MAX_DROPOUT) {
		/* in order, with permissible gap */
		if (seq < s->max_seq) {
			/*
			* Sequence number wrapped - count another 64K cycle.
			*/
			s->cycles += RTP_SEQ_MOD;
		}
		s->max_seq = seq;
	} else if (udelta <= RTP_SEQ_MOD - MAX_MISORDER) {
		/* the sequence number made a very large jump */
		if (seq == s->bad_seq) {
			/*
			* Two sequential packets -- assume that the other side
			* restarted without telling us so just re-sync
			* (i.e., pretend this was the first packet).
			*/
			init_seq(s, seq);
		}
		else {
			s->bad_seq = (seq + 1) & (RTP_SEQ_MOD-1);
			return 0;
		}
	} else {
	   /* duplicate or reordered packet */
	}
	s->received++;
	return 1;
}

char *rtp_write_sdes(char *b, u_int32 src, int argc,
		rtcp_sdes_type_t type[], char *value[],
		int length[])
{
	rtcp_sdes_t *s = (rtcp_sdes_t *)b;
	rtcp_sdes_item_t *rsp;
	int i;
	int len;
	int pad;

	/* SSRC header */
	s->src = src;
	rsp = &s->item[0];

	/* SDES items */
	for (i = 0; i < argc; i++) {
		rsp->type = type[i];
		len = length[i];
		if (len > RTP_MAX_SDES) {
			/* invalid length, may want to take other action */
			len = RTP_MAX_SDES;
		}
		rsp->length = len;
		memcpy(rsp->data, value[i], len);
		rsp = (rtcp_sdes_item_t *)&rsp->data[len];
	}

	/* terminate with end marker and pad to next 4-octet boundary */
	len = ((char *) rsp) - b;
	pad = 4 - (len & 0x3);
	b = (char *) rsp;
	while (pad--) *b++ = RTCP_SDES_END;

	return b;
}

void rtp_read_sdes(rtcp_t *r)
{
	int count = r->common.count;
	rtcp_sdes_t *sd = &r->r.sdes;
	rtcp_sdes_item_t *rsp, *rspn;
	rtcp_sdes_item_t *end = (rtcp_sdes_item_t *)
			       ((u_int32 *)r + r->common.length + 1);
	source *s;

	while (--count >= 0) {
		rsp = &sd->item[0];
		if (rsp >= end) break;
		s = find_member(sd->src);

		for (; rsp->type; rsp = rspn ) {
			rspn = (rtcp_sdes_item_t *)((char*)rsp+rsp->length+2);
			if (rspn >= end) {
				rsp = rspn;
				break;
			}
			member_sdes(s, rsp->type, rsp->data, rsp->length);
		}
		sd = (rtcp_sdes_t *)
		((u_int32 *)sd + (((char *)rsp - (char *)sd) >> 2)+1);
	}
	if (count >= 0) {
	/* invalid packet format */
	}
}