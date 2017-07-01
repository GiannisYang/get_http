#ifndef DEF1_H
#define DEF1_H

#include "def0.h"

struct ether_header {
	u_int8_t	ether_dhost[6]; // dest of ether address
	u_int8_t	ether_shost[6]; // src of ..
	u_int16_t	ether_type; // type of ether
};

typedef u_int32_t	in_addr_t; // def of ip address

struct ip_header {
#ifdef WORDS_BIGENDIAN
	u_int8_t		ip_version : 4, /* 版本 */
	ip_header_length : 4;			/* 首部长度 */
#else
	u_int8_t		ip_header_length : 4, ip_version : 4;
#endif
	u_int8_t		ip_tos;/* 服务质量 */
	u_int16_t		ip_length;/* 总长度 */
	u_int16_t		ip_id;/* 标识 */
	u_int16_t		ip_off;/* 偏移 */
	u_int8_t		ip_ttl;/* 生存时间 */
	u_int8_t		ip_protocol;/* 协议类型 */
	u_int16_t		ip_checksum;/* 校验和 */
	in_addr	ip_souce_address;
	in_addr	ip_destination_address;
};

struct tcp_header {
	u_int16_t	tcp_source_port;
	u_int16_t	tcp_destination_port;

	u_int32_t	tcp_acknowledgement;/* 序列号 */
	u_int32_t	tcp_ack;/* 确认号 */
#ifdef WORDS_BIGENDIAN
	u_int8_t	tcp_offset : 4,

	/* 偏移 */
	tcp_reserved : 4;

	/* 保留 */
#else
	u_int8_t	tcp_reserved : 4,

	/* 保留 */
	tcp_offset : 4;

	/* 偏移 */
#endif
	u_int8_t	tcp_flags;/* 标志 */
	u_int16_t	tcp_windows;/* 窗口大小 */
	u_int16_t	tcp_checksum;/* 校验和 */
	u_int16_t	tcp_urgent_pointer;/* 紧急指针 */
};

#endif
