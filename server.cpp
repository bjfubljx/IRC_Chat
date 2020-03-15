#include "command.hpp"
#include "socket.hpp"
#include "io.hpp"

#define FLAGS_WAIT_TASK_EXIT 1
typedef struct
{
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    size_t addrLen;
    char buffer[BUFFER_SIZE];
    struct epoll_event ev;
    struct epoll_event events[MAXEPOLL_FD];
    int epollfd;
    int socketfd;
} server_t;

//memset(buffer, 0, sizeof(char)*BUFFER_SIZE);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


int initServer(void *arg)
{
    server_t *server;
    server = (server_t *)arg;
    server->serverAddr.sin_family = AF_INET;
    server->serverAddr.sin_port = htons(SERVER_PORT);
    server->serverAddr.sin_addr.s_addr = INADDR_ANY;
    server->addrLen = sizeof(struct sockaddr);
    bzero(&server->serverAddr.sin_zero, 8);
    server->addrLen = sizeof(struct sockaddr);
    memset(server->buffer, 0, sizeof(server->buffer));
    memset(server->events, 0, sizeof(server->events));
    server->socketfd = 0;
    server->epollfd = 0;
}

void *closeServer(void *arg)
{
    server_t *server;
    server = (server_t *)arg;
    close(server->socketfd);
    close(server->epollfd);
}

void *handleServer(void *arg)
{
    server_t *server;
    server = (server_t *)arg;
    int nfds = epoll_wait(server->epollfd, server->events, sizeof(server->events), -1);
    if (nfds < 0)
    {
        perror("Epoll_Wait Error.");
        exit(-1);
    }
    int connfd;
    /* to itrate loop*/
    char welcome[BUFFER_SIZE] = {" Welcome To Connect Server!"};
    for (int i = 0; i < nfds; i++)
    {
        /* if socket fd has been changed.*/
        if (server->events[i].data.fd == server->socketfd)
        {
            int connfd = Accept(server->socketfd, (struct sockaddr *)&server->clientAddr, (socklen_t *)&server->addrLen);
            clientfd.push_back(connfd);
            fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL) | O_NONBLOCK);
            server->ev.events = EPOLLIN | EPOLLET;
            server->ev.data.fd = connfd;
            if(epoll_ctl(server->epollfd, EPOLL_CTL_ADD, server->ev.data.fd, &server->ev) < 0)
            {
                perror(" Epoll_Ctl Error.");
                exit(-1);
            }
            send(connfd, welcome, sizeof(welcome), 0);
        }
        else
        {
            recv(server->events[i].data.fd, server->buffer, sizeof(server->buffer), 0);
            std::cout << "meassge from client: " << server->buffer << std::endl;
            conduct_message(server->buffer, server->events[i].data.fd);
        }
    }
}

void *startServer(void *arg)
{
    server_t *server;
    server = (server_t *)arg;
    Socket(server->socketfd);
    Bind(server->socketfd, (struct sockaddr *)&server->serverAddr, server->addrLen);
    Listen(server->socketfd, 5);
    /* socket can be read*/
    server->ev.events = EPOLLIN | EPOLLET;
    server->ev.data.fd = server->socketfd;
    /*create epoll*/
    server->epollfd = epoll_create(MAXNUM_CLIENT);
    if (server->epollfd < 0)
    {
        perror("Epoll_Create Error.");
        exit(-1);
    }

    /* register event*/
    if (epoll_ctl(server->epollfd, EPOLL_CTL_ADD, server->socketfd, &server->ev) < 0)
    {
        perror("Epoll_Ctl Error.");
        exit(-1);
    }
    int rc, fd, n;
    while (1)
    {
        handleServer(server);
        //threadpool_add_task((threadpool_t *)pool, &threadServer, (server_t*)&server);
    }

    closeServer(server);
}

int main(int argc, char *argv[])
{
    server_t *server;
    initServer(&server);
    void *pool;
    //threadpool_add_task((threadpool_t *)pool, &startServer, (server_t *)&server);
        //threadpool_destory((threadpool_t *)pool);
    startServer(&server);
    return 0;
}
