#ifndef STRUCT_H
#define STRUCT_H
#include <sys/socket.h>
#include <sys/types.h>

struct pseudo_header
{
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t tcp_length;
};

struct ip_port_element
{
	char ip[20];
	int port;
	int port_incoming_pkt_from_server; // = port_incoming_pkt_from_server
	unsigned char fin_ack;			   // to check that both server and client exchange fin ack -> only used in client column side
};

struct server_element
{
	char ip[25];
	int port;
};

#endif