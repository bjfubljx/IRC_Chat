#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "command.hpp"
#include "socket.hpp"

int main(int argc, char *argv[])
{
    struct sockaddr_in serverAddr;
    size_t addrLen = sizeof(struct sockaddr_in);
    memset(&serverAddr, 0, sizeof(struct sockaddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    int socketfd = 0, epollfd = 0, nfds = 0;
    struct epoll_event events[2];

    Socket(socketfd);
    Connect(socketfd, (struct sockaddr *)&serverAddr, addrLen);
    /*
        pipe_fd[0]: read by farther process
        pipe_fd[1]: written by child process
    */
    int pipe_fd[2] = {0};
    if (pipe(pipe_fd) < 0)
    {
        perror("Pipe Error.");
        exit(-1);
    }

    epollfd = epoll_create(MAXNUM_CLIENT);
    if (epollfd < 0)
    {
        perror("Epoll_Create Error.");
        exit(-1);
    }

    addEpollfd(epollfd, socketfd, true);
    addEpollfd(epollfd, pipe_fd[0], true);
    char buffer[BUFFER_SIZE] = {0};
    char send_buffer[BUFFER_SIZE] = {0};
    char recv_buffer[BUFFER_SIZE] = {0};
    int pid;
    int clientStatus = 1;
    if ((pid = fork()) < 0)
    {
        perror("Fork Error.");exit(-1);
    }
    else if (pid == 0)
    {
        /*child process*/
        close(pipe_fd[0]); //child process majors in writting message,so it closes read pipe
        while (clientStatus)
        {
            /*child process writes message to pipe*/
            memset(buffer, 0, sizeof(buffer));
            fgets(buffer, sizeof(buffer), stdin);

            int ret = write(pipe_fd[1], buffer, sizeof(send_buffer));
            if ( ret < 0)
            {
                perror(" Child Write Pipe Error");exit(-1);
            }

        }
    }
    else{
        /*farther process*/
        close(pipe_fd[1]); //farther peocess majors in reading message,it closes writting pipe
        while (clientStatus)
        {
            nfds = epoll_wait(epollfd, events, sizeof(events), -1);
            /* handle nfds*/
            for (int i = 0; i < nfds; i++)
            {
                memset(recv_buffer, 0, sizeof(recv_buffer));
                /*message from server*/
                if (events[i].data.fd == socketfd)
                {
                    int ret = recv(socketfd, recv_buffer, sizeof(recv_buffer), 0);
                    // ret= 0 服务端关闭
                    if (ret == 0)
                    {
                        std::cout << "Server closed connection: " << socketfd << std::endl;
                        close(socketfd);
                        clientStatus = 0;
                        //break;
                    }
                    else
                    {
                        std::cout << "message from server: " << recv_buffer << std::endl;
                    }
                }
                else 
                {
                    /* child writes pipe happend,farther handles it and sends it to server*/
                    int ret = read(pipe_fd[0], recv_buffer, sizeof(recv_buffer));
                    if(ret == 0)
                        clientStatus = 0;
                    else
                        send(socketfd, recv_buffer, sizeof(recv_buffer), 0);
                }
            }
        }
    }

    if(pid){
       //关闭父进程的管道和sock
        close(pipe_fd[0]);
        close(socketfd);
    }else{
        //关闭子进程的管道
        close(pipe_fd[1]);
    }

    return 0;
}
