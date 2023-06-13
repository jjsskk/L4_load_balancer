# L4_load_balancer

simple load balancer

0. 이 프로그램은 방화벽이나 외부 공유기의 routing, NAT설정등의 방해요소를 제거하기 위해 모든 process가 같은 homenetwork에서만 test되었습니다. 

1. 최소 2개 이상의 ubuntu(linux) 환경이 필요함. test는 라즈베리 파이에 ubuntu 설치후 진행되었습니다.

ubuntu 1 : client, unbuntu 2 , 3 : loadbalancer, server(이 두 프로세스는 한 환경에 다 돌려도 무방하지만 2개의 환경에서 따로돌리는것 을 추천)

2. ubuntu에 gcc와 g++이 설치

3. loadbalancer 환경에선 반드시 다음의 명령어를 제일 먼저 실행 시킬것

sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP

4. server
gcc chat_serv.c -o chat_serv (빈드시 3개의 서버를 돌릴것)

./chat_serv port1

./chat_serv port2

./chat_serv port3

5.load balancer

g++ loadbalancerLobin.cpp -o loadbalancerLobin

sudo ./loadbalancerLobin SourceIp Server1Ip Port1 Server2Ip Port2 Server3Ip Port3

6. client( check하고 싶은만큼 client 여려명 돌리면됨)

gcc chat_clnt -o chat_clnt

./chat_clnt LoadbalancerIp 20000(Loadbalancer port fixed) ID







 
