#ifndef GLOVAL_VARIABLE 
#define GLOVAL_VARIABLE
#include <list>
#include <set>
#include <vector>
// #define port_incoming_pkt_from_server 20001

extern std::set<int> portlist_for_server;                              // assign one port to one server. never use reused port for some servers
extern std::list<std::list<struct ip_port_element *> *> ip_port_table; // for Network Address Translation(NAT)
extern std::vector<struct server_element *> server_table;              // store server ip and port
extern std::vector<bool> server_status_table;                          // store server status(true = alive or false = dead)
extern unsigned short round_robin_index;
extern unsigned short numberOfServer;

#endif