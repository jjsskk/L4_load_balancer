#include <iostream>
#include <string.h>
#include <time.h>
#include <string>
#include <thread>
#include <unistd.h>
#include "loadbalancerLobin.h"
#include "analyzePkt.h"
#include "healthCheck.h"
#include "send.h"


// Before run this code, execute the command below
// $ sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP

using namespace std;;

void ReceiveFrom(int sock, char *buffer, size_t buffer_length, struct sockaddr_in *src, struct sockaddr_in *dst)
{
	unsigned short dst_port = 0;
	int received;

	while (1)
	{
		received = recvfrom(sock, buffer, buffer_length, 0, NULL, NULL);
		if (received <= 0)
		{
			dst_port = 0;
			perror("ReceiveFrom()");
			// exit(EXIT_FAILURE);
			break;
		}
		memcpy(&dst_port, buffer + 22, sizeof(dst_port));
		if (ntohs(dst_port) == client_port_incoming_pkt) // packet from client
		{

			printf("destination port: %d\n", ntohs(dst_port));
			printf("Successfully receive pkt from client\n");
			SendToServer(sock, buffer, received, src, dst);
		}
		// Contains() in loadbalancerLobin.cpp
		if (Contains(portlist_for_server, (int)ntohs(dst_port))) // check if packet is from server.
		{
			printf("destination port: %d\n", ntohs(dst_port));
			printf("Successfully receive pkt from server\n");
			SendToClient(sock, buffer, received, src, dst);
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc < 6 || argc % 2 != 0)
	{
		printf("Usage: %s <Source IP> <Server1 IP> <Server1 port> <Server2 IP> <Server2 port>....\n", argv[0]);
		return 1;
	}

	// assign healthcheck to thread
	numberOfServer = (argc / 2) - 1;
	for (int i = 2; i < argc; i += 2)
	{
		struct server_element *element = new struct server_element;
		// GetIPAddress() in loadbalancerLobin.cpp
		const char *ip = GetIPAddress(argv[i]); // DNS and Check if ip is valid 
		strcpy(element->ip, ip);
		element->port = atoi(argv[i + 1]);
		server_table.push_back(element);
		delete[] ip;
		server_status_table.push_back(true);
		thread thread(HealthCheckServer, element->ip, atoi(argv[i + 1]), (i / 2) - 1);
		thread.detach();
	}

	srand(time(NULL));

	// The IPPROTO_TCP constant is used to indicate that the socket will use the TCP protocol
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	if (sock == -1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// Source IP
	const char *ip = GetIPAddress(argv[1]); // DNS and Check if ip is valid 
	printf("srcIP : %s\n", ip);

	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(client_port_incoming_pkt); // client_port_incomgin_pkt -> 20000 in send.h
	// saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (inet_pton(AF_INET, ip, &saddr.sin_addr) != 1)
	{
		perror("Source IP configuration failed\n");
		exit(EXIT_FAILURE);
	}
	delete[] ip;

	// Destination IP and Port
	struct sockaddr_in daddr;
	daddr.sin_family = AF_INET;

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

	ReceiveFrom(sock, recvbuf, sizeof(recvbuf), &saddr, &daddr);

	close(sock);
	return 0;
}