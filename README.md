# L4_load_balancer
simple load balancer

sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP

./chat_clnt <LB ip> 20000(LB PORT FIXED) ID

g++ loadbalancerLobin.cpp -o loadbalancerLobin
sudo ./loadbalancerLobin <LB source IP> <Server1 ip> <port> <Server2 ip> <port> <Server3 ip> <port>

sudo ./chat_serv <server1 port>
sudo ./chat_serv <server2 port>
sudo ./chat_serv <server3 port>
