/* ulogd_MAC.c, Version $Revision: 1.1 $
 *
 * ulogd logging interpreter for MAC addresses, TIME, etc.
 *
 * (C) 2000 by Harald Welte <laforge@sunbeam.franken.de>
 * This software is released under the terms of GNU GPL
 *
 * $Id: ulogd_BASE.c,v 1.1 2000/08/02 08:51:15 laforge Exp laforge $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ulogd.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/icmp.h>

ulog_iret_t *_interp_mac(ulog_packet_msg_t *pkt)
{
	unsigned char *p;
	int i;
	char *buf;
	ulog_iret_t *ret;
	
	if (pkt->mac_len)
	{
		buf = (char *) malloc(3 * pkt->mac_len + 1);
		*buf = 0;

		p = pkt->mac;
		for (i = 0; i < pkt->mac_len; i++, p++)
			sprintf(buf, "%s%02x%c", buf, *p, i==pkt->mac_len-1 ? ' ':':');
		ret = alloc_ret(ULOGD_RET_STRING,"raw.mac.addr");
		ret->value.ptr = buf;
		return ret;

	}
	return NULL;
}

ulog_iret_t *_interp_time(ulog_packet_msg_t *pkt)
{
	ulog_iret_t *ret, *ret2;
	
	ret = alloc_ret(ULOGD_RET_UINT32, "oob.time.sec");
	ret2 = alloc_ret(ULOGD_RET_UINT32, "oob.time.usec");

	ret->value.ui32 = pkt->timestamp_sec;
	ret->next = ret2;

	ret2->value.ui32 = pkt->timestamp_usec;
	
	return ret;
}

ulog_iret_t *_interp_prefix(ulog_packet_msg_t *pkt)
{
	ulog_iret_t *ret;
	
	ret = alloc_ret(ULOGD_RET_STRING, "oob.prefix");
	ret->value.ptr = malloc(sizeof(pkt->prefix));
	strcpy(ret->value.ptr, pkt->prefix);
	
	return ret;
}

ulog_iret_t *_interp_mark(ulog_packet_msg_t *pkt)
{
	ulog_iret_t *ret;

	ret = alloc_ret(ULOGD_RET_UINT32, "oob.mark");
	ret->value.ui32 = pkt->mark;

	return ret;	
}

ulog_iret_t *_interp_iphdr(ulog_packet_msg_t *pkt)
{
	ulog_iret_t *ret, *ret2;
	struct iphdr *iph = (struct iphdr *) pkt->payload;

	ret = alloc_ret(ULOGD_RET_IPADDR, "ip.hdr.saddr");
	ret->value.ui32 = ntohl(iph->saddr);

	ret->next = ret2 = alloc_ret(ULOGD_RET_IPADDR, "ip.hdr.daddr");
	ret2->value.ui32 = ntohl(iph->daddr);

	ret2 = ret2->next = alloc_ret(ULOGD_RET_UINT8, "ip.hdr.protocol");
	ret2->value.ui8 = iph->protocol;

	ret2 = ret2->next = alloc_ret(ULOGD_RET_UINT8, "ip.hdr.tos");
	ret2->value.ui8 = ntohs(iph->tos);

	ret2 = ret2->next = alloc_ret(ULOGD_RET_UINT8, "ip.hdr.ttl");
	ret2->value.ui8 = iph->ttl;

	ret2 = ret2->next = alloc_ret(ULOGD_RET_UINT16, "ip.hdr.tot_len");
	ret2->value.ui16 = ntohs(iph->tot_len);

	ret2 = ret2->next = alloc_ret(ULOGD_RET_UINT8, "ip.hdr.ihl");
	ret2->value.ui8 = iph->ihl;

	return ret;
}

ulog_iret_t *_interp_tcphdr(ulog_packet_msg_t *pkt)
{
	struct iphdr *iph = (struct iphdr *) pkt->payload;
	struct tcphdr *tcph = (struct tcphdr *) (iph + iph->ihl);
	ulog_iret_t *ret, *ret2;

	if (iph->protocol != IPPROTO_TCP)
		return NULL;
	
	ret = alloc_ret(ULOGD_RET_UINT16, "tcp.hdr.sport");
	ret->value.ui16 = ntohs(tcph->source);

	ret->next = ret2 = alloc_ret(ULOGD_RET_UINT16, "tcp.hdr.sport");
	ret->value.ui16 = ntohs(tcph->dest);

	ret2 = ret2->next = alloc_ret(ULOGD_RET_UINT32, "tcp.hdr.seq");
	ret->value.ui32 = ntohl(tcph->seq);
	
	ret2 = ret2->next = alloc_ret(ULOGD_RET_UINT32, "tcp.hdr.ack_seq");
	ret->value.ui32 = ntohl(tcph->ack_seq);
	
	return ret;
}

ulog_iret_t *_interp_icmp(ulog_packet_msg_t *pkt)
{
	struct iphdr *iph = (struct iphdr *) pkt->payload;
	struct icmphdr *icmph = (struct icmphdr *) (iph + iph->ihl);
	ulog_iret_t *ret, *ret2;

	if (iph->protocol != IPPROTO_ICMP)
		return NULL;
	
	ret = alloc_ret(ULOGD_RET_UINT8, "icmp.hdr.type");
	ret->value.ui8 = icmph->type;

	return ret;

}


static ulog_interpreter_t base_ip[] = { 

	{ NULL, "raw.mac", &_interp_mac },
	{ NULL, "oob.time", &_interp_time },
	{ NULL, "oob.prefix", &_interp_prefix },
	{ NULL, "oob.mark", &_interp_mark },
	{ NULL, "ip.hdr", &_interp_iphdr },
	{ NULL, "tcp.hdr", &_interp_tcphdr },
	{ NULL, "icmp.hdr", &_interp_icmp },
	{ NULL, "", NULL }, 
};
void _base_reg_ip(void)
{
	ulog_interpreter_t *ip = base_ip;
	ulog_interpreter_t *p;

	for (p = ip; p->interp; p++)
		register_interpreter(p);

}


void _init(void)
{
	_base_reg_ip();
}