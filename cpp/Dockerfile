FROM alpine:latest

WORKDIR /app

COPY . .

RUN apk update \
    && apk add build-base

RUN g++ -o main loadbalancerLobin.cpp -std=c++11

ENTRYPOINT ["./main"]