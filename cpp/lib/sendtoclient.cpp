#include <send.h>

void sendToClient(int sock, char *buffer, int received_len, struct sockaddr_in *src, struct sockaddr_in *dst)
{
	struct iphdr *iph = (struct iphdr *)buffer;
	struct tcphdr *tcph = (struct tcphdr *)(buffer + sizeof(struct iphdr));
	struct pseudo_header psh; // required to calculate checksum

	char tempip[20] = "0.0.0.0";
	int tempport = 0;

	std::list<std::list<struct ip_port_element *> *>::iterator ip_port_table_index;

	for (auto it = ip_port_table.begin(); it != ip_port_table.end(); it++)
	{
		auto element_client = (*it)->front();
		auto element_server = (*it)->back();
		printf("\nserver : %s %d\n", element_server->ip, element_server->port);
		printf("\n%s %d\n", inet_ntoa(*(struct in_addr *)&iph->saddr), ntohs(tcph->source));
		printf("\nclient : %s %d\n", element_client->ip, element_client->port);

		// if( element_server -> port == ntohs(tcph->source) && !strcmp(element_server->ip , inet_ntoa(*(struct in_addr *)&iph->saddr) ) )
		if (element_server->port_incoming_pkt_from_server == ntohs(tcph->dest))
		{
			ip_port_table_index = it;
			strcpy(tempip, element_client->ip);
			tempport = element_client->port;
			// port_incoming_pkt_from_server = (*element_server)->port_incoming_pkt_from_server;
			break;
		}
		// printf("  %d",*it);
	}

	if (0 == tempport)
		return;

	if (inet_pton(AF_INET, tempip, &(dst->sin_addr)) != 1) // set server ip addr
	{
		perror("client destination IP and Port configuration failed");
		exit(EXIT_FAILURE);
	}

	src->sin_port = htons(client_port_incoming_pkt);
	dst->sin_port = htons(tempport);


	iph->saddr = src->sin_addr.s_addr;
	iph->daddr = dst->sin_addr.s_addr;
	iph->check = 0;

	tcph->source = src->sin_port;
	tcph->dest = dst->sin_port;
	tcph->check = 0;

	// TCP pseudo header for checksum calculation
	psh.source_address = src->sin_addr.s_addr;					 // 진짜값을 넣어줌 오로지 tcp checksum을 위해 사용
	psh.dest_address = dst->sin_addr.s_addr;					 // 아직 ipheader까지 안갔으니까
	psh.placeholder = 0;										 // reserved ->0으로 채움
	psh.protocol = (iph->protocol);								 // tcp ->6
	psh.tcp_length = htons(received_len - sizeof(struct iphdr)); // data(payload) + tcpheader

	// fill pseudo packet
	char *pseudogram = (char *)malloc(received_len - sizeof(struct iphdr) + sizeof(struct pseudo_header));
	memcpy(pseudogram, (char *)&psh, sizeof(struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header), tcph, received_len - sizeof(struct iphdr));

	// in loadbalancerLobin.hpp
	tcph->check = CheckSum((unsigned short *)pseudogram, received_len - sizeof(struct iphdr) + sizeof(struct pseudo_header)); // tcp checksum
	iph->check = CheckSum((unsigned short *)buffer, received_len);															  // ip checksum

	int sent;
	if (tempport != 0)
	{
		if ((sent = sendto(sock, buffer, received_len, 0, (struct sockaddr *)dst, sizeof(struct sockaddr))) == -1)
		{
			perror("sendto()");
			exit(EXIT_FAILURE);
		}
		else
		{
			printf("Successfully sent %d bytes to client!\n", sent);
			auto element_client = (*ip_port_table_index)->front();
			auto element_server = (*ip_port_table_index)->back();
			if ((element_client->fin_ack) == 2) // each other (Server and client) exchange fin ack
			{
				printf("\nclient <%s,%d> terminated \n", element_client->ip, element_client->port);
				portlist_for_server.erase(element_server->port_incoming_pkt_from_server);
				// std::list<std::list<struct ip_port_element*>*>::iterator ip_port_table_index;
				free(element_client);
				free(element_server);
				delete ((*ip_port_table_index));
				ip_port_table.erase(ip_port_table_index);
				printf("Current NAT table length : %d\n", (int)ip_port_table.size());
			}
			if (tcph->fin == 1 && tcph->ack == 1)
			{
				(element_client->fin_ack)++;
			}
		}
	}

	free(pseudogram);
}