#include <send.h>

void SendToServer(int sock, char *buffer, int received_len, struct sockaddr_in *src, struct sockaddr_in *dst)
{
	struct iphdr *iph = (struct iphdr *)buffer;
	struct tcphdr *tcph = (struct tcphdr *)(buffer + sizeof(struct iphdr));
	struct pseudo_header psh; // required to calculate checksum
	char client_addr[20];
	// analyzeIPDatagram(buffer, received_len);
	// analyzeTCPSegment(buffer, received_len);
	// set destination  = server ip

	char tempip[20] = "0.0.0.0";
	int tempport = 0;
	int port_incoming_pkt_from_server;
	int client_port_from_client;

	list<list<struct ip_port_element *> *>::iterator ip_port_table_index;
	strcpy(client_addr, inet_ntoa(*(struct in_addr *)&iph->saddr));
	printf("client ip addr =  %s\n", client_addr);

	if (tcph->syn == 1 && tcph->ack == 0) // tcp 3 handshake를 위해 처음 연결시도하는 pkt
	{
		RoundRobin(src, dst, &tempport, (int)ntohs(tcph->source), inet_ntoa(*(struct in_addr *)&iph->saddr)); // in loadbalancerLobin.cpp
	}
	else
	{

		// NAT를 위해 ip_port_table에 해당 pkt의 tcp header source port와 ip header src ip가 있는지 검색
		for (auto it = ip_port_table.begin(); it != ip_port_table.end(); it++)
		{
			auto element_client = (*it)->front();
			auto element_server = (*it)->back();

			if (element_client->port == ntohs(tcph->source) && !strcmp(element_client->ip, inet_ntoa(*(struct in_addr *)&iph->saddr)))
			{
				ip_port_table_index = it;
				strcpy(tempip, element_server->ip);
				tempport = element_server->port;
				port_incoming_pkt_from_server = element_server->port_incoming_pkt_from_server;
				break;
			}
		}

		if (0 == tempport) // tempport 0 -> NAT를 위해 사용되는 ip_port_table에 없는 pkt -> 출처를 알 수없는 PKT(악성 패킷일수도)
			return;

		if (inet_pton(AF_INET, tempip, &(dst->sin_addr)) != 1) // set server ip addr
		{
			perror("Server destination IP and Port configuration failed");
			exit(EXIT_FAILURE);
		}

		src->sin_port = htons(port_incoming_pkt_from_server);
		dst->sin_port = htons(tempport);
	}

	printf("   |-Source IP address: %s\n", inet_ntoa(*(struct in_addr *)&iph->saddr));
	printf("   |-Destination IP address: %s\n", inet_ntoa(*(struct in_addr *)&iph->daddr));
	iph->saddr = src->sin_addr.s_addr;
	iph->daddr = dst->sin_addr.s_addr;
	printf("   |-Network Address Translation---------\n");
	printf("   |-Source IP address: %s\n", inet_ntoa(*(struct in_addr *)&iph->saddr));
	printf("   |-Destination IP address: %s\n", inet_ntoa(*(struct in_addr *)&iph->daddr));

	iph->check = 0;

	client_port_from_client = tcph->source;
	printf("client port : %d\n", ntohs(client_port_from_client));

	printf("   |-Source Port: %u\n", ntohs(tcph->source));
	printf("   |-Destination Port: %u\n", ntohs(tcph->dest));
	printf("   |-Network Address Translation---------\n");
	tcph->source = src->sin_port;
	tcph->dest = dst->sin_port; // real port num for server to operate application
	printf("   |-Source Port: %u\n", ntohs(tcph->source));
	printf("   |-Destination Port: %u\n", ntohs(tcph->dest));
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

	// in loadbalancerLobin.cpp
	tcph->check = CheckSum((unsigned short *)pseudogram, received_len - sizeof(struct iphdr) + sizeof(struct pseudo_header)); // tcp checksum

	iph->check = CheckSum((unsigned short *)buffer, received_len); // ip checksum

	// analyzeIPDatagram(buffer, received_len);
	// analyzeTCPSegment(buffer, received_len);

	// sleep(1);
	int sent;
	if (0 != tempport)
	{
		if ((sent = sendto(sock, buffer, received_len, 0, (struct sockaddr *)dst, sizeof(struct sockaddr))) == -1)
		{
			perror("sendto()");
			exit(EXIT_FAILURE);
		}
		else
		{

			printf("Successfully sent %d bytes to server!\n", sent);
			if (tcph->syn == 0)
			{

				auto element_client = (*ip_port_table_index)->front();
				auto element_server = (*ip_port_table_index)->back();
				if ((element_client->fin_ack) == 2) // each other (Server and client) exchange fin ack
				{
					printf("\nclient <%s,%d> terminated \n", element_client->ip, element_client->port);
					portlist_for_server.erase(element_server->port_incoming_pkt_from_server);
					// list<list<struct ip_port_element*>*>::iterator ip_port_table_index;
					delete (element_client);
					delete (element_server);
					delete (*ip_port_table_index);
					ip_port_table.erase(ip_port_table_index);
					printf(" Current NAT table length : %d\n", (int)ip_port_table.size());
				}
				if (tcph->fin == 1 && tcph->ack == 1)
				{
					(element_client->fin_ack)++;
				}
			}
		}
	}
	free(pseudogram);
}