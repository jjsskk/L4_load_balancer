# L4_load_balancer

## RAW SOCKET 기반 로드밸런서 (CPP)

- 이 프로그램은 방화벽이나 외부 공유기의 routing, NAT설정 등의 방해요소를 제거하기 위해 모든 프로세스가 같은 home network 내에서 테스트되었습니다.

- 최소 2개 이상의 ubuntu(linux) 환경이 필요합니다. 데모에서는 라즈베리파이에 ubuntu 설치후 진행했습니다.

- ubuntu 1 : client, unbuntu 2 , 3 : load balancer, server(이 두 프로세스는 한 환경에 다 돌려도 무방하지만 2개의 환경에서 따로 돌리는것을 추천)

- `gcc`와 `g++` 설치 필요

- 로드밸런서 환경에선 반드시 다음의 명령어를 제일 먼저 실행 시킬 것(make file 참조)

```
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP
```
- 자세한 설명은 가이드 라인 파일 참조

### 1. Server(make file 실행해도됨)

(반드시 3개의 서버를 돌릴것)

```
gcc chat_serv.c -o chat_serv -pthread
```

```
./chat_serv <PORT>
```

### 2. Load Balancer(make file 실행해도됨)

```
g++ -std=c++11 loadbalancerLobin.cpp -o loadbalancerLobin
```

```
sudo ./loadbalancerLobin <SourceIP> <SERVER_IP> <SERVER_PORT> <SERVER_IP2> <SERVER_PORT2> <SERVER_IP3> <SERVER_PORT3>
```

### 3. Client(make file 실행해도됨)

- 테스트 하고 싶은 클라이언트의 수만큼 프로그램 실행
- 로드밸런서 포트 번호는 `20000`으로 고정

```
gcc chat_clnt.c -o chat_clnt -pthread
```

```
./chat_clnt LoadbalancerIp 20000 <NICKNAME>
```

---

## TCP 소켓 인터페이스 기반 로드밸런서 (GOLANG)

### 무엇인가요?

고언어로 작성된 TCP 소켓 인터페이스 기반 로드밸런서입니다. 클라이언트 접속시 최소 연결(Least Connection) 방식으로 서버의 연결 수를 체크하고, 가장 적은 연결 수를 가진 서버에 클라이언트를 연결합니다. 서버의 상태는 헬스체크를 통해 주기적으로 확인하며 클라이언트의 세션이 유휴 상태인 경우 로드밸런서는 세션을 종료하고 해당 서버가 관리하는 대상에서 제외합니다.

### 어떻게 실행하나요?

~~여러 터미널에 독립적으로 실행함으로써 로컬에서 테스트 가능합니다. 예를 들어 클라이언트 3대, 로드 밸런서 1대, 서버 3대인 경우 7개의 터미널이 필요합니다.~~

도커 컴포즈를 이용해 로컬에 가상의 서버 3대와 로드밸런서 1대를 생성하고, 호스트에서 클라이언트 프로그램을 실행함으로써 테스트할 수 있습니다.

1.먼저 도커 컴포즈를 이용해 서버와 로드밸런서를 구성하는 컨테이너를 생성합니다.

```
docker-compose -f docker-compose.go.yml up
```

2. `docker ps` 명령어를 이용해 컨테이너가 잘 생성되었는지 확인합니다.

3. 클라이언트 프로그램을 실행합니다. 테스트 하고 싶은 클라이언트의 수만큼 터미널을 생성해 실행합니다. 로드밸런서 포트 번호는 `20000`으로 고정되어 있습니다.

```
gcc chat_clnt.c -o client
```

```
./client 127.0.0.1 20000 <NICKNAME>
```

## 결과 예시

### 초기 연결

```
l4_load_balancer-l4lb-golang-1  | Connection from: 172.18.0.4:8090
l4_load_balancer-l4lb-golang-1  | Connection from: 172.18.0.2:8091
l4_load_balancer-l4lb-golang-1  | Connection from: 172.18.0.3:8092
l4_load_balancer-l4lb-golang-1  |
l4_load_balancer-l4lb-golang-1  | Load balancer listening on port :20000
```

### 헬스체크

```
l4_load_balancer-l4lb-golang-1  | Received 12 bytes from server: HEALTH CHECK
l4_load_balancer-l4lb-golang-1  |
l4_load_balancer-l4lb-golang-1  | Received 12 bytes from server: HEALTH CHECK
l4_load_balancer-l4lb-golang-1  |
l4_load_balancer-l4lb-golang-1  | Received 12 bytes from server: HEALTH CHECK
l4_load_balancer-l4lb-golang-1  |
l4_load_balancer-l4lb-golang-1  |
l4_load_balancer-l4lb-golang-1  | The server is alive: 172.18.0.2:8091
l4_load_balancer-l4lb-golang-1  |
l4_load_balancer-l4lb-golang-1  | The server is alive: 172.18.0.3:8092
l4_load_balancer-l4lb-golang-1  |
l4_load_balancer-l4lb-golang-1  | The server is alive: 172.18.0.4:8090
```

### 클라이언트 연결

```
l4_load_balancer-l4lb-golang-1  | 172.18.0.1:49384 -> 172.18.0.4:8090
l4_load_balancer-l4lb-golang-1  | Now the server manages the following connections:
l4_load_balancer-l4lb-golang-1  | -  172.18.0.1:49384
l4_load_balancer-l4lb-golang-1  | server: 172.18.0.4:8090
l4_load_balancer-l4lb-golang-1  |
l4_load_balancer-l4lb-golang-1  | Connection from 172.18.0.1:49384
```

### 타임아웃

```
l4_load_balancer-l4lb-golang-1  | Timeout: read tcp 172.18.0.5:20000->172.18.0.1:49384: i/o timeout
l4_load_balancer-l4lb-golang-1  | Disconnect 172.18.0.1:49384 from server 172.18.0.4:8090
```
