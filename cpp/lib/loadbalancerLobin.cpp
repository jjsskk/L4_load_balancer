#include <algorithm>
#include <string.h>
#include <sys/socket.h>
#include <list>
#include <arpa/inet.h>
#include <netdb.h>
#include "loadbalancerLobin.h"
#include "globalVariable.h"
#include "struct.h"

template <typename T>
bool Contains(std::set<T> &listOfElements, const T &element)
{
	// Find the iterator if element in list
	auto it = std::find(listOfElements.begin(), listOfElements.end(), element); // # include <algorithm>
	// return if iterator points to end or not. It points to end then it means element
	//  does not exists in list
	return it != listOfElements.end();
}

int CheckPortList()
{
	int port_incoming_pkt_from_server;
	do
	{
		port_incoming_pkt_from_server = rand() % 65535;
		// port_incoming_pkt_from_server = 10000;

	} while (Contains(portlist_for_server, port_incoming_pkt_from_server));

	portlist_for_server.insert(port_incoming_pkt_from_server);
	// printf("\nport list used :");
	// for (auto it = portlist_for_server.begin(); it != portlist_for_server.end(); it++)
	// 	printf("  %d", *it);
	// printf("\n");
	return port_incoming_pkt_from_server;
}

void RoundRobin(struct sockaddr_in *src, struct sockaddr_in *dst, int *tempport, int tcph_port, char *iph_saddr)
{
	char server_ip[20];
	int server_port;
	int port_incoming_pkt_from_server = CheckPortList(); // make new source port for new client
	struct ip_port_element *element_client = (struct ip_port_element *)malloc(sizeof(struct ip_port_element));
	struct ip_port_element *element_server = (struct ip_port_element *)malloc(sizeof(struct ip_port_element));
	strcpy(element_client->ip, iph_saddr); // based on host order
	element_client->port = tcph_port;	   // based on host order
	element_client->fin_ack = 0;

	// round robin algorithm
	while (1)
	{
		if (server_status_table[round_robin_index] == true)
			break;
		round_robin_index = round_robin_index + 1;
		round_robin_index = round_robin_index % numberOfServer;
	}
	auto selected_server = server_table.at(round_robin_index);
	strcpy(server_ip, selected_server->ip);
	server_port = selected_server->port;
	round_robin_index = round_robin_index + 1;
	round_robin_index = round_robin_index % numberOfServer;

	// round robin algorithm

	strcpy(element_server->ip, server_ip); // server ip to send pkt to //based on host order
	element_server->port = server_port;	   // server port to send pkt to // based on host order
	element_server->port_incoming_pkt_from_server = port_incoming_pkt_from_server;
	printf("ip port :%s %d\n", element_client->ip, element_client->port);

	std::list<struct ip_port_element *> *element = new std::list<struct ip_port_element *>;
	element->push_back(element_client);
	element->push_back(element_server);
	ip_port_table.push_back(element);

	if (inet_pton(AF_INET, server_ip, &(dst->sin_addr)) != 1) // set server ip addr
	{
		perror("Server destination IP and Port configuration failed");
		exit(EXIT_FAILURE);
	}

	*tempport = port_incoming_pkt_from_server;
	src->sin_port = htons(port_incoming_pkt_from_server);
	dst->sin_port = htons(server_port);
}

const char *GetIPAddress(const char *hostname) // DNS and Check if ip is valid
{
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;		 // Use AF_INET6 for IPv6
	hints.ai_socktype = SOCK_STREAM; // Use SOCK_DGRAM for UDP

	int status = getaddrinfo(hostname, nullptr, &hints, &res);
	if (status != 0)
	{
		std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
		exit(EXIT_FAILURE);
		return "";
	}

	// Extract the IP address from the first result
	char *ip = new char[INET_ADDRSTRLEN];
	inet_ntop(res->ai_family, &((struct sockaddr_in *)res->ai_addr)->sin_addr, ip, INET_ADDRSTRLEN);

	freeaddrinfo(res);

	return ip;
}

unsigned short CheckSum(unsigned short *buffer, unsigned short size) // datagram or pseudogram is recognized as unsigned short buffer
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