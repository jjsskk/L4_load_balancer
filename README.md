# L4_load_balancer

## RAW SOCKET 기반 로드밸런서 (CPP)

- 이 프로젝트는 방화벽이나 외부 공유기의 routing, NAT설정 등의 방해요소를 제거하기 위해 home network 환경에서 테스트되었습니다.

- loadbalancer가 주기적으로 서버의 상태를 체크합니다.(healthcheck) 

- 부하 분산 알고리즘은 라운드 로빈 방식을 사용했으며 헬스체크를 통해 죽은 서버를 확인한다면 그쪽으로는 부하를 분산하지 않고 살아있는 서버로만 부하가 분산되도록 설계했습니다.

- ./shared/chat_clnt.c(client), ./shared/chat.serv.c(server), ./cpp/로드밸런서 관련 모든 cpp 파일 모두 POSIX Socket 기반으로 작성 되었습니다.

-  ubuntu(linux) 환경이 필요합니다. ~~데모에서는 라즈베리파이에 ubuntu 설치후 진행했습니다.~~

- ubuntu 1 : client, unbuntu 2 : load balancer, unbuntu 3 : server( 모든 작업을 1개의 로컬 환경(ubuntu)에서 테스트해도 무방합니다)

- 필요 라이브러리: `gcc`, `g++`, `cmake`

```
kho@kho-desktop:~/cpp$ cmake -version
cmake version 3.28.1

CMake suite maintained and supported by Kitware (kitware.com/cmake).
```


- 도커를 사용하지 않는 터미널 기반 로드밸런서 환경에선 반드시 다음의 명령어를 실행시켜야 하며 정상적으로 실행이 되는지 확인해야 합니다.
- 도커를 사용할 경우 dockerfile을 통해 container에서 명령어가 자동으로 실행이 됩니다.
- 명령어 실행이 되지 않는다면  RAW SOCKET 기반 로드밸런서가 제대로 동작하지 않습니다. 

```
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP
```

- 스크린샷을 이용한 자세한 시연 과정 설명은 가이드 라인 파일 참조

### Linux(Ubuntu) 환경에서 테스트 

### 1. Server(shared 폴더)

- shared 폴더로 이동 후 여러 터미널을 이용해 2개이상의 서버를 테스트 할 수 있습니다.
- 컴파일 -> make

```
gcc chat_serv.c -o chat_serv -pthread
```

```
./chat_serv <PORT>
```

### 2. Load Balancer(cpp 폴더)
- cpp 폴더로 이동해서 다음의 명령어들을 수행해주세요.(빌드된 모든 파일은 build 폴더에 저장되며 실행파일은 bin 폴더에 생성됩니다.)

```
mkdir build && cd build 
cmake .. 
make
cd bin
```
```
sudo ./loadbalancerLobin <SourceIP> <SERVER_IP> <SERVER_PORT> <SERVER_IP2> <SERVER_PORT2>...
```

### 3. Client(shared 폴더)

- 여러 터미널을 이용해 테스트 하고 싶은 클라이언트의 수만큼 프로그램 실행 할 수 있습니다.
- 로드밸런서 포트 번호는 `20000`으로 고정 (1개의 로컬환경에서 모든 프로세스를 테스트 할 경우 서버 포트는 20000을 사용하면 안됨)
- 컴파일 -> make

```
gcc chat_clnt.c -o chat_clnt -pthread
```

```
./chat_clnt LoadbalancerIp 20000 <NICKNAME>
```

### Docker compose 이용한 테스트 

- POSIX Socket을 사용했기 때문에 window 환경에서는 이 프로젝트를 테스트 할 수 없습니다.

- 어떠한 환경(OS)에서도 이 프로젝트를 테스트 할 수 있도록 docker compose를 이용해 client, server, loadbalancer를 container 형태로 실행시켜 이 모든 container들을 한번에 관리 할 수 있는 환경을 구축했습니다 (Docker Compose Orchestration).

- `docker`, `docker compose` 설치가 필요합니다.


### 1. Server, Load Balancer

- CMD(terminal)를 열고 yml 파일이 있는 위치에서 다음의 명령어를 실행 시키면 로드벨런서와 3개의 서버가 container 형태로 자동으로 실행이 됩니다.

```
docker-compose -f docker-compose.cpp.yml up
```
- `docker ps` 명령어를 이용해 컨테이너가 잘 생성되었는지 확인합니다.

- 다음의 명령어를 실행시키면 로드벨런서와 3개의 서버 container들이 자동으로 종료됩니다.

```
docker-compose -f docker-compose.cpp.yml down
```

### 2. Client

- 로드밸런서 포트 번호는 `20000`으로 고정

    ###  a. window 환경

    - docker compose를 통해 생성된 client container에서 테스트 할 수 있습니다.

    - CMD창을 열어 다음의 명령어를 통해 client continer에 접속 합니다.

    ```
    docker exec -it RootFolderNAME_client_1 /bin/bash
    ```

    - 일반적으로 RootFolderNAME은 yml file이 위치한 폴더 이름이나 혹시나 실행이 안된다면 `docker ps`를 통해 client container name을 확인하고 하고 실행하시면 됩니다.

    ```
    C:\Users\kjs>docker ps
    CONTAINER ID   IMAGE      COMMAND                  CREATED          STATUS          PORTS                      NAMES
    741941327c0a   client     "sleep infinity"         11 seconds ago   Up 9 seconds                               finalproject_client_1
    eab5d222ef2b   l4lb-cpp   "./entrypoint.sh ./m…"   14 seconds ago   Up 11 seconds   0.0.0.0:20000->20000/tcp   finalproject_l4lb-cpp_1
    cb1cc338fa4e   server     "./serv 8092"            14 seconds ago   Up 11 seconds   0.0.0.0:1053->8092/tcp     finalproject_server3_1
    21e0e23dc61a   server     "./serv 8091"            14 seconds ago   Up 11 seconds   0.0.0.0:1052->8091/tcp     finalproject_server2_1
    a4f953537736   server     "./serv 8090"            14 seconds ago   Up 10 seconds   0.0.0.0:1054->8090/tcp     finalproject_server1_1
    ```

    - 다음 명령어를 통해서 로드벨런서 컨테이너에 접속합니다. 

    - 이 명령어는 nickname을 제외한 다른 argument는 그대로 사용해야 합니다.
    -  l4lb-cpp -> yml 파일에서 정의한 로드벨런서 서비스 이름으로 도커 컴포즈 내부 DNS에 의해 자동으로 로드 밸런서 컨테이너 IP주소로 변환됩니다.

    ```
    ./client l4lb-cpp 20000 <nickname>
    ```
    - 접속하고 싶은 client 수 만큼 위의 과정을 반복합니다.

    ###  b. macos, linux 환경

    -외부 혹은 로컬 macos, linux 환경에서 client 프로그램을 실행시켜 도커 컴포즈가 실행중인 환경에 접속 가능합니다. (yml파일을 통해 로드밸런서 도커 포트(20000)와 로컬 호스트 포트(20000) 동기화함)

    ```
    gcc chat_clnt.c -o client -pthread
    ```

    ```
    ./client LoadbalancerIp 20000 <NICKNAME>
    ```

---

## TCP 소켓 인터페이스 기반 로드밸런서 (GOLANG)

### 무엇인가요?

고언어로 작성된 TCP 소켓 인터페이스 기반 로드밸런서입니다. 클라이언트 접속시 최소 연결(Least Connection) 방식으로 서버의 연결 수를 체크하고, 가장 적은 연결 수를 가진 서버에 클라이언트를 연결합니다. 서버의 상태는 헬스체크를 통해 주기적으로 확인하며 클라이언트의 세션이 유휴 상태인 경우 로드밸런서는 세션을 종료하고 해당 서버가 관리하는 대상에서 제외합니다.

### 어떻게 실행하나요?

~~여러 터미널에 독립적으로 실행함으로써 로컬에서 테스트 가능합니다. 예를 들어 클라이언트 3대, 로드 밸런서 1대, 서버 3대인 경우 7개의 터미널이 필요합니다.~~

도커 컴포즈를 이용해 로컬에 가상의 서버 3대와 로드밸런서 1대를 생성하고, 호스트에서 클라이언트 프로그램을 실행함으로써 테스트할 수 있습니다.

1. 먼저 도커 컴포즈를 이용해 서버와 로드밸런서를 구성하는 컨테이너를 생성합니다.

```
docker-compose -f docker-compose.go.yml up
```

2. `docker ps` 명령어를 이용해 컨테이너가 잘 생성되었는지 확인합니다.

3. 클라이언트 프로그램을 실행합니다. 테스트 하고 싶은 클라이언트의 수만큼 터미널을 생성해 실행합니다. 로드밸런서 포트 번호는 `20000`으로 고정되어 있습니다.

#### macos, linux 환경

-외부 혹은 로컬 macos, linux 환경에서 client 프로그램을 실행시켜 도커 컴포즈가 실행중인 환경에 접속 가능합니다. (yml파일을 통해 로드밸런서 도커 포트(20000)와 로컬 호스트 포트(20000) 동기화함)

```
gcc chat_clnt.c -o client
```

```
./client 127.0.0.1 20000 <NICKNAME>
```
- 도커 컴포즈가 로컬호스트에서 돌아가고있다면 127.0.0.1 사용해도 되지만 외부환경이라면 그에 맞는 로드밸런서 ip를 입력해야 합니다

#### window 환경

- docker compose를 통해 생성된 client container에서 테스트 할 수 있습니다.

- CMD창을 열어 다음의 명령어를 통해 client continer에 접속 합니다.

```
docker exec -it RootFolderNAME_client_1 /bin/bash
```

- 일반적으로 RootFolderNAME은 yml file이 위치한 폴더 이름이나 혹시나 실행이 안된다면 `docker ps`를 통해 client container name을 확인하고 하고 실행하시면 됩니다.

```
C:\Users>docker ps
CONTAINER ID   IMAGE         COMMAND                  CREATED         STATUS         PORTS                      NAMES
c776d110907d   client        "sleep infinity"         9 minutes ago   Up 9 minutes                              finalproject_client_1
9f82afeedefe   l4lb-golang   "./main server1 8090…"   9 minutes ago   Up 9 minutes   0.0.0.0:20000->20000/tcp   finalproject_l4lb-golang_1
ad416195046d   server        "./serv 8091"            9 minutes ago   Up 9 minutes   0.0.0.0:13051->8091/tcp    finalproject_server2_1
e24ce673f075   server        "./serv 8090"            9 minutes ago   Up 9 minutes   0.0.0.0:13045->8090/tcp    finalproject_server1_1
a3a609333e0c   server        "./serv 8092"            9 minutes ago   Up 9 minutes   0.0.0.0:13046->8092/tcp    finalproject_server3_1
```

- 다음 명령어를 통해서 로드벨런서 컨테이너에 접속합니다. 

- 이 명령어는 nickname을 제외한 다른 argument는 그대로 사용해야 합니다. ( l4lb-golang -> yml 파일에서 정의한 로드벨런서 서비스 이름으로 도커 컴포즈 내부 DNS에 의해 자동으로 로드 밸런서 컨테이너 IP주소로 변환됩니다. )

```
./client l4lb-golang 20000 <nickname>
```
- 접속하고 싶은 client 수 만큼 위의 과정을 반복합니다.


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

---

## Reference

- shared 폴더에 있는 client와 server 파일은 [윤성우의 열혈 TCP/IP 소켓 프로그래밍](https://www.yes24.com/Product/Goods/3630373) 책에 있는 예제를 참조했습니다.
