#ifndef __SOCKET_HPP
#define __SOCKET_HPP

#include "head.hpp"
#define SERVER_IP "127.0.0.1"   //default server ip address
#define SERVER_PORT 8888
#define BUFFER_SIZE 2048
#define MAXNUM_CLIENT   100
#define MAXEPOLL_FD 100


int Socket(int &socketfd)
{
    if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { 
        perror("Socket Error."); exit(-1);
    }
    return socketfd;
}

int Bind(int &socketfd, const struct sockaddr *serverAddr, size_t &addrLen)
{
    int n = 0, on = 1;
    if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        perror("SetsockOpt Error."); exit(-1);
    }
    if((n = bind(socketfd, serverAddr, addrLen)) < 0)
    {
        perror("Bind Error."); exit(-1);
    }
    return 1;
}

int Listen(int &socketfd, int listenNums)
{
    int n = 0;
    if((n = listen(socketfd, listenNums)) < 0)
    {
        perror("Listen Error"); exit(-1);
    }
    return n;
}

int Accept(int &socketfd, struct sockaddr *clientAddr, socklen_t *addrLen)
{
    int n = 0;
    if((n = accept(socketfd, clientAddr, addrLen)) < 0)
    {
        perror("Accept Error."); exit(-1);
    }
    return n;
}

int Connect(int &socketfd,struct sockaddr *serverAddr, size_t &addrLen)
{
    int n = 0;
    if((n = connect(socketfd, serverAddr, addrLen)) < 0)
    {
        perror("Connect Error."); exit(-1);
    }
    return n;
}
#endif // !__SOCKET_HPP
