# L4_load_balancer

simple load balancer

최소 2~3개의 ubuntu(linux) 환경이필요 -> ubuntu 1 :client , ubuntu 1~2 : loadbalancer, server (같은 환경에서 돌려도 가능) 

load balancer 환경에서 test전에 반드시 실핼 시킬것 : sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP

./chat_clnt LBIp 20000(LB PORT FIXED) ID


g++ loadbalancerLobin.cpp -o loadbalancerLobin

sudo ./loadbalancerLobin LBsourceIP Server1Ip port1 Server2Ip port2 Server3Ip port3


./chat_serv server1port

./chat_serv server2port

./chat_serv server3port
