#include <globalVariable.h>
// #define port_incoming_pkt_from_server 20001

std::set<int> portlist_for_server;                              // assign one port to one server. never use reused port for some servers
std::list<std::list<struct ip_port_element *> *> ip_port_table; // for Network Address Translation(NAT)
std::vector<struct server_element *> server_table;              // store server ip and port
std::vector<bool> server_status_table;                          // store server status(true = alive or false = dead)
unsigned short round_robin_index = 0;
unsigned short numberOfServer = 0;

