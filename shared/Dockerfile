FROM gcc:latest

WORKDIR /app

COPY ./chat_serv.c .

RUN gcc -o serv chat_serv.c

ENTRYPOINT ["./serv"]