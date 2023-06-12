# L4_load_balancer
simple load balancer

sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP


./chat_clnt LBIp 20000(LB PORT FIXED) ID


g++ loadbalancerLobin.cpp -o loadbalancerLobin

sudo ./loadbalancerLobin LBsourceIP Server1Ip port1 Server2Ip port2 Server3Ip port3


./chat_serv server1port

./chat_serv server2port

./chat_serv server3port
