#include <iostream>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

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