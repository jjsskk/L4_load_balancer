#ifndef HEALTHCHECK_H
#define HEALTHCHECK_H
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>

int ConnectWithTimeout(int sock, struct sockaddr_in *serv_addr, int sec);
int GetCurrentSec();
void HealthCheckServer(char *ip, int port, int server_status_table_index);

#endif