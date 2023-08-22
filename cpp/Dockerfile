FROM alpine:latest

WORKDIR /app

COPY . .

RUN chmod +x ./entrypoint.sh

RUN apk update \
    && apk add build-base iptables

RUN g++ -o main loadbalancerLobin.cpp -std=c++11

ENTRYPOINT ["./entrypoint.sh", "./main"]