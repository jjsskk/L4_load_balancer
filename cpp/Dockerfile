FROM alpine:3.18

WORKDIR /app

# 소스 코드 및 CMakeLists.txt 복사
COPY . .

RUN chmod +x ./entrypoint.sh

# CMake 빌드
RUN apk update \
    && apk add build-base iptables cmake git\
    && mkdir build \
    && cd build \
    && cmake .. \
    && make

ENTRYPOINT ["./entrypoint.sh", "./build/bin/loadbalancerLobin"]
