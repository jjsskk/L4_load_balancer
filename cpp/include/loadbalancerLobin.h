#ifndef LOBIN_H
#define LOBIN_H
#include <set>
#include <iostream>

template <typename T>
bool contains(std::set<T> &listOfElements, const T &element);
int check_portlist();

void roundRobin(struct sockaddr_in *src, struct sockaddr_in *dst, int *tempport, int tcph_port, char *iph_saddr);

const char *getIPAddress(const char *hostname);

unsigned short checksum(unsigned short *buffer, unsigned short size);

#endif