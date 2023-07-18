#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <list>
#include <set>
#include <string>
#include <algorithm>
#include <vector>
#include <netdb.h>

// Before run this code, execute the command below
// $ sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP

#define DATAGRAM_LEN 4096
#define client_port_incoming_pkt 20000
// #define port_incoming_pkt_from_server 20001

char server_ip[20];
int port_incoming_pkt_from_server; // = port_incoming_pkt_from_server
int client_port_from_client;
char client_addr[20];
int num = 0;
std::set<int> portlist_for_server; // assign one port to one server. never use reused port for some servers
int server_port;

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
	char ip[20];
	int port;
};
std::list<std::list<struct ip_port_element *> *> ip_port_table; // for Network Address Translation(NAT)
std::vector<struct server_element *> server_table;				// store server ip and port
int round_robin_index = 0;

#include "loadbalancerLobin.hpp"

const char* getIPAddress(const char* hostname) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // Use AF_INET6 for IPv6
    hints.ai_socktype = SOCK_STREAM; // Use SOCK_DGRAM for UDP
    
    int status = getaddrinfo(hostname, nullptr, &hints, &res);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
		exit(EXIT_FAILURE);
        return "";
    }
    
    // Extract the IP address from the first result
    char* ip = new char[INET_ADDRSTRLEN];
    inet_ntop(res->ai_family, &((struct sockaddr_in*)res->ai_addr)->sin_addr, ip, INET_ADDRSTRLEN);
    
    freeaddrinfo(res);
    
    return ip;
}

void analyzeIPDatagram(char *buffer, int size)
{
	struct iphdr *ip = (struct iphdr *)(buffer); // ip header를 바로 가리키게 됨

	printf("IP Datagram\n");
	printf("   |-Version: %d\n", ip->version);
	printf("   |-Internet Header Length: %d DWORDS or %d Bytes\n", ip->ihl, ip->ihl * 4);
	printf("   |-Type of Service: %d\n", ip->tos);
	printf("   |-Total Length: %d Bytes\n", ntohs(ip->tot_len));
	printf("   |-Identification: %d\n", ntohs(ip->id));
	printf("   |-Flags: ");
	if (ip->frag_off & IP_DF)
		printf("Don't Fragment ");
	if (ip->frag_off & IP_MF)
		printf("More Fragments ");
	printf("\n");
	printf("   |-Fragment Offset: %d\n", ip->frag_off & IP_OFFMASK);
	printf("   |-Time to Live: %d\n", ip->ttl);
	printf("   |-Protocol: %d\n", ip->protocol);
	printf("   |-Checksum: %d\n", ntohs(ip->check));
	// printf("   |-Checksum: %d\n", ip->check); //원래는 이게 맞는듯 checksum은 변환 안함
	printf("   |-Source IP address: %s\n", inet_ntoa(*(struct in_addr *)&ip->saddr));
	printf("   |-Destination IP address: %s\n", inet_ntoa(*(struct in_addr *)&ip->daddr));
}

void analyzeTCPSegment(char *buffer, int size)
{
	struct tcphdr *tcp = (struct tcphdr *)(buffer + sizeof(struct iphdr)); // tcp header를 가리키게됨

	printf("TCP Segment\n");
	printf("   |-Source Port: %u\n", ntohs(tcp->source));
	printf("   |-Destination Port: %u\n", ntohs(tcp->dest));
	printf("   |-Sequence Number: %u\n", ntohl(tcp->seq)); // ntohl
	printf("   |-Acknowledgment Number: %u\n", ntohl(tcp->ack_seq));
	printf("   |-Data Offset: %d DWORDS or %d Bytes\n", tcp->doff, tcp->doff * 4);
	printf("   |-Flags: ");
	if (tcp->urg)
		printf("URG ");
	if (tcp->ack)
		printf("ACK ");
	if (tcp->psh)
		printf("PSH ");
	if (tcp->rst)
		printf("RST ");
	if (tcp->syn)
		printf("SYN ");
	if (tcp->fin)
		printf("FIN ");
	printf("\n");
	printf("   |-Window Size: %d\n", ntohs(tcp->window));
	printf("   |-Checksum: %d\n", ntohs(tcp->check));
	// printf("   |-Checksum: %d\n", tcp->check); //원래는 이게 맞는듯 checksum은 변환 안함
	printf("   |-Urgent Pointer: %d\n", ntohs(tcp->urg_ptr));
}

unsigned short checksum(unsigned short *buffer, unsigned short size) // datagram or pseudogram is recognized as unsigned short buffer
{
	unsigned long checksum = 0;
	while (size > 1) // add all header fileds + data
	{
		checksum += *buffer++;			// increment address positon by unsigned short(2byte) becase buffer is a kind of unsigned short array
		size -= sizeof(unsigned short); // 2byte (16bit)
	}
	if (size)
		checksum += *(unsigned char *)buffer; // error cause -> checksum += *(unsigned short*)buffer;(previous code)

	// we have to originally wrap around each time we add 16 bits,
	// but even if we do this, it has the same effect.
	checksum = (checksum >> 16) + (checksum & 0xffff); // wrap aroud -> carry + only valid 16bit checksum (remainder values except 16bit are 0) that is extract from original checksum(unsigned long)
	checksum = (checksum >> 16) + (checksum & 0xffff); // wrap around again because remainder carray may exist
	return (unsigned short)(~checksum);				   // ons's complment
}

#include "sendtoserver.cpp"
#include "sendtoclient.cpp"

void receive_from(int sock, char *buffer, size_t buffer_length, struct sockaddr_in *src, struct sockaddr_in *dst)
{
	unsigned short dst_port = 0;
	int received;

	while (1)
	{
		received = recvfrom(sock, buffer, buffer_length, 0, NULL, NULL);
		if (received <= 0)
		{
			dst_port = 0;
			perror("receive_from()");
			// exit(EXIT_FAILURE);
			break;
		}
		memcpy(&dst_port, buffer + 22, sizeof(dst_port));
		if (ntohs(dst_port) == client_port_incoming_pkt) // packet from client
		{

			printf("Successfully received bytes: %d\n", received);
			printf("destination port: %d\n", ntohs(dst_port));
			printf("receive pkt from client\n");
			sendtoserver(sock, buffer, received, src, dst);
		}
		// if( ntohs(dst_port) == port_incoming_pkt_from_server)
		if (contains(portlist_for_server, (int)ntohs(dst_port))) // packet from server
		{
			printf("Successfully  received bytes: %d\n", received);
			printf("destination port: %d\n", ntohs(dst_port));
			printf("receive pkt from server\n");
			sendtoclient(sock, buffer, received, src, dst);
			// printf("send to client\n");
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc != 8)
	{
		printf("Usage: %s <Source IP> <Server1 IP> <Server1 port> <Server2 IP> <Server2 port> <Server3 IP> <Server3 port>\n", argv[0]);
		return 1;
	}

	struct server_element *element1 = (struct server_element *)malloc(sizeof(struct server_element));
	const char *ip1 = getIPAddress(argv[2]);
	strcpy(element1->ip, ip1);
	element1->port = atoi(argv[3]);
	struct server_element *element2 = (struct server_element *)malloc(sizeof(struct server_element));
	const char *ip2 = getIPAddress(argv[4]);
	strcpy(element2->ip, ip2);
	element2->port = atoi(argv[5]);
	struct server_element *element3 = (struct server_element *)malloc(sizeof(struct server_element));
	const char *ip3 = getIPAddress(argv[6]);
	strcpy(element3->ip, ip3);
	element3->port = atoi(argv[7]);
	server_table.push_back(element1);
	server_table.push_back(element2);
	server_table.push_back(element3);

	delete []ip1;
	delete []ip2;
	delete []ip3;

	for (auto it = server_table.begin(); it != server_table.end(); it++)
		printf("server ip :%s  port : %d operating \n", (*it)->ip, (*it)->port);

	srand(time(NULL));
	// port_incoming_pkt_from_server = rand() % 65535;
	// portlist_for_server.insert(port_incoming_pkt_from_server);
	// The IPPROTO_TCP constant is used to indicate that the socket will use the TCP protocol
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	if (sock == -1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// Source IP
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(client_port_incoming_pkt); // random client port (원래는 os가 자동 할당하지만 복잡)
	if (inet_pton(AF_INET, argv[1], &saddr.sin_addr) != 1)
	{
		perror("Source IP configuration failed\n");
		exit(EXIT_FAILURE);
	}

	// Destination IP and Port
	strcpy(server_ip, argv[2]);
	server_port = atoi(argv[3]);
	printf("%s", server_ip);
	struct sockaddr_in daddr;
	daddr.sin_family = AF_INET;
	// daddr.sin_port = htons(atoi(argv[2]));
	// if (inet_pton(AF_INET, argv[2], &daddr.sin_addr) != 1)
	// {
	// 	perror("Destination IP and Port configuration failed");
	// 	exit(EXIT_FAILURE);
	// }

	// Tell the kernel that headers are included in the packet
	int one = 1;
	const int *val = &one;
	/*
	When you send a packet using a raw socket with the IP_HDRINCL option, you provide the IP packet, including the IP header, to the operating system.
	The operating system then encapsulates the IP packet within the appropriate link layer header, such as Ethernet, before sending it out on the network
	Using only raw sockets without additional libraries or lower-level packet capture mechanisms, it is not possible to directly reconstruct the link layer header.
	Raw sockets typically operate at the IP layer and above, allowing you to construct and manipulate IP and higher-level headers
	*/
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) == -1)
	{
		perror("setsockopt(IP_HDRINCL, 1)");
		exit(EXIT_FAILURE);
	}

	char recvbuf[DATAGRAM_LEN];
	// char *recvbuf = (char*)calloc(DATAGRAM_LEN , sizeof(char));

	receive_from(sock, recvbuf, sizeof(recvbuf), &saddr, &daddr);

	close(sock);
	return 0;
}