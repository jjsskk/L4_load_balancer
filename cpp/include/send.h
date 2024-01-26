#ifndef SEND_H
#define SEND_H

#include <list>
#include <set>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <string.h>
#include "struct.h"
#include "loadbalancerLobin.h"
#include "globalVariable.h"

#define DATAGRAM_LEN 4096
#define client_port_incoming_pkt 20000

void sendtoserver(int sock, char *buffer, int received_len, struct sockaddr_in *src, struct sockaddr_in *dst);
void sendtoclient(int sock, char *buffer, int received_len, struct sockaddr_in *src, struct sockaddr_in *dst);
#endif