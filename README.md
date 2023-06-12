# L4_load_balancer
simple load balancer

sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP
g++ loadbalancerLobin.cpp -o loadbalancerLobin 
