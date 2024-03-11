#ifndef LOBIN_H
#define LOBIN_H
#include <set>
#include <iostream>

template <typename T>
bool Contains(std::set<T> &listOfElements, const T &element);
int CheckPortList();

void RoundRobin(struct sockaddr_in *src, struct sockaddr_in *dst, int *tempport, int tcph_port, char *iph_saddr);

const char *GetIPAddress(const char *hostname);

unsigned short CheckSum(unsigned short *buffer, unsigned short size);

#endif