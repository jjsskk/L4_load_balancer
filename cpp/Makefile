all: 
	g++ -std=c++11 loadbalancerLobin.cpp -o loadbalancerLobin
	sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP


clean:
	rm loadbalancerLobin 	