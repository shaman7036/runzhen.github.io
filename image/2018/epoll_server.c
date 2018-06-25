#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/epoll.h>  
#include <netdb.h>  
#include <string.h>  
#include <stdio.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <stdlib.h>  
#include <errno.h>  
#include <sys/wait.h>  
#include <arpa/inet.h>

#define PROCESS_NUM 5  
#define MAX_EVENT  64 

void epoll_event_loop(int fd, int efd,
                      struct epoll_event *eventsp)
{
    while (1) {
        int n, i;
        n = epoll_wait(efd, eventsp, MAX_EVENT, -1);
        printf("child [%d] wake up from epoll_wait\n", getpid());

        /* sleep to make the "thundering herd" happen */
        sleep(1);

        for (i = 0; i < n; i++) {
            if ((eventsp[i].events & EPOLLERR) || (eventsp[i].events & EPOLLHUP) ||
                (!(eventsp[i].events & EPOLLIN))) {
                printf("Error: efd is not ready for read\n");
                continue;
            } else if (eventsp[i].data.fd == fd) {
                struct sockaddr in_addr;
                socklen_t in_len;
                int infd;

                in_len = sizeof(in_addr);
                infd = accept(fd, &in_addr, &in_len);

                if (infd == -1) {
                    printf("Error: accept failed\n");
                } else {
                    printf("child %d accept successed\n", getpid());
                }
            } else {
                printf("Error: eventsp[%d].data.fd = %d is not we want\n", i, eventsp[i].data.fd);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("\n\t Usage: %s IP PORT\n\n", argv[0]);
        return -1;
    }

    /* create socket and bind */
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
    int pid, ret;  
    struct sockaddr_in serveraddr;  
    serveraddr.sin_family = AF_INET;  
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);  
    serveraddr.sin_port = htons(atoi(argv[2]));  
    bind(fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));  

    /* make socket non blocking */
    int flags = fcntl(fd, F_GETFL, 0);

    flags |= O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);

    /* listen on fd */
    listen(fd, 1024);
    
    /* epoll */
    struct epoll_event event;
    struct epoll_event *eventsp;

    int efd = epoll_create(MAX_EVENT);
    event.data.fd = fd; /* socket fd */
    event.events = EPOLLIN;

    /* add socket fd to epoll efd */
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event); 

    eventsp = calloc(MAX_EVENT, (sizeof(struct epoll_event)));
    printf("fork %d children ... \n", PROCESS_NUM);

    for (int i = 0; i < PROCESS_NUM; i++) {
        int pid = fork();
        if (pid == 0) {
            epoll_event_loop(fd, efd, eventsp);
        }
    }

    int status;
    wait(&status);
    free(eventsp);
    close(fd);
    return 0;
}
