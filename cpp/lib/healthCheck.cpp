#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include "globalVariable.h"

int ConnectWithTimeout(int sock, struct sockaddr_in *serv_addr, int sec)
{
    int err;
    socklen_t len;
    fd_set writefds;
    struct timeval timeout;
    int flags;
    flags = fcntl(sock, F_GETFL);
    flags = (flags | O_NONBLOCK);
    if (fcntl(sock, F_SETFL, flags) != 0)
    {
        perror("fcntl() error");
        return -1;
    }
    if (connect(sock, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) != 0) // return without blocking
    {
        if (errno != EINPROGRESS)
        {
            perror("connect() error");
            return -1;
        }
    }
    // struct sockaddr_in sin;
    // socklen_t len = sizeof(sin);
    // if (getsockname(sock, (struct sockaddr *)&sin, &len) == -1)
    //     perror("getsockname");
    // else
    //     printf("port number %d\n", ntohs(sin.sin_port));

    timeout.tv_sec = sec;
    timeout.tv_usec = 0;
    FD_ZERO(&writefds);
    FD_SET(sock, &writefds);
    if (select(sock + 1, NULL, &writefds, NULL, &timeout) <= 0) // client connection also require write pkt(data)
    {
        perror("connection timeout");
        return -1;
    }
    len = sizeof(err);
    getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&err, &len);
    if (err)
    {
        perror("fcntl() error");
        return -1;
    }
    fcntl(sock, F_GETFL);
    flags = (flags & ~O_NONBLOCK);
    if (fcntl(sock, F_SETFL, flags) != 0)
    {
        perror("fcntl() error");
        return -1;
    }
    return 0;
}

int GetCurrentSec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

void HealthCheckServer(char *ip, int port, int server_status_table_index)
{

    int retry_number = 3;
    int retry_number_maximum = 3;
    int interval = 5;
    int response_time = 2;
    int sock;
    bool server_status = true;
    struct sockaddr_in serv_addr;
    while (1)
    {
        sock = socket(PF_INET, SOCK_STREAM, 0);

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(ip);
        serv_addr.sin_port = htons(port);

        int t1 = GetCurrentSec();
        if (ConnectWithTimeout(sock, &serv_addr, response_time) < 0)
        {
            if (server_status)
            {
                printf("IP: <%s>, Port : <%d> Server is bad \n", ip, port);
                //   std::cout<<server_status_table[server_status_table_index]<<std::endl;
            }
            else
            {
                printf("IP: <%s>, Port : <%d> Server is dead \n", ip, port);
                if (server_status_table[server_status_table_index] != false)
                    server_status_table[server_status_table_index] = false;
                //   std::cout<<server_status_table[server_status_table_index]<<std::endl;
            }

            close(sock);
            if (retry_number > 0)
                retry_number--;
        }
        else
        {
            printf("IP : <%s>, Port : <%d> Server is alive\n", ip, port);
            close(sock);
            retry_number = retry_number_maximum;
            server_status = true;
            if (server_status_table[server_status_table_index] != true)
                server_status_table[server_status_table_index] = true;
            // std::cout<<server_status_table[server_status_table_index]<<std::endl;
        }
        int t2 = GetCurrentSec();
        if (retry_number == 0)
            server_status = false;
        int d = t2 - t1;
        // printf("%d sec \n", d);
        sleep(interval - d);
    }
}