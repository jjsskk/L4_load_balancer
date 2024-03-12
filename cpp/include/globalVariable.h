#ifndef GLOVAL_VARIABLE 
#define GLOVAL_VARIABLE
#include <list>
#include <set>
#include <vector>

using namespace std;

extern set<int> portlist_for_server;                              // assign one port to one server. never use reused port for some servers
extern list<list<struct ip_port_element *> *> ip_port_table;      // for Network Address Translation(NAT)
extern vector<struct server_element *> server_table;              // store server ip and port
extern vector<bool> server_status_table;                          // store server status(true = alive or false = dead) for healthcheck
extern unsigned short round_robin_index;
extern unsigned short numberOfServer;

#endif