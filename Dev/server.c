#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define PORT 5000

#define BACKLOG 10

int sockfd;

fd_set read_fds, write_fds;

int fdmax;

struct buffers {
    int buf_cnt;
    int buf_size;
    char *buf;
} buffers[16];

int buf_size = 64;

int int_socket() {
    struct sockaddr_in my_addr;
    int yes=1;
    
    FD_ZERO(&read_fds);
    
    memset(&my_addr, 0, sizeof(my_addr));

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    
    if(setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(int)) < 0) {
        perror("setsockopt");
        exit(1);
    }
    
    //unsigned long one = 1;
    if(ioctl(sockfd, FIONBIO, &yes) < 0) {
        perror("ioctl");
        exit(1);
    }
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    
    FD_SET(sockfd, &read_fds);
    
    fdmax = sockfd;
    printf("xo: Server socket: %d\n",sockfd);
    return 0;
}

int connect_client() {
    int sin_size;
    int new_fd;
    struct sockaddr_in their_addr;
    
    sin_size = sizeof(struct sockaddr_in);

    if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr,&sin_size)) == -1) { 
        perror("accept");
    } else {
        FD_SET(new_fd, &read_fds);
        if (new_fd > fdmax) {
            fdmax = new_fd;
        }
        buffers[new_fd].buf_size = buf_size;
        buffers[new_fd].buf = malloc(buf_size);
        printf("xo: new connection from %s on socket %d\n", inet_ntoa(their_addr.sin_addr), new_fd);
    }
    
    return 0;
}
int console_input() {
    int n;
    int arg1;
    char *arg2;
    
    printf("console in\n");
    arg2 = malloc(16);
    
    buffers[0].buf_cnt = read(0, buffers[0].buf, buffers[0].buf_size);
    
    sscanf(buffers[0].buf, "%d:%s", &arg1 , arg2 );
    
    if ( FD_ISSET( arg1, &read_fds)) {
        if ( (send(arg1, buffers[0].buf, buffers[0].buf_cnt, 0)) < 0 )
            perror("send");
        else
            buffers[0].buf_cnt = 0;
    }
        
    return 0;
}

int recieve(int rfd) {
    char buf[32];
    int num_bytes;
    if ( rfd == sockfd ) {
        connect_client();
        return 0;
    } else if ( rfd == 0 ) {
        console_input();
        return 0;
    }    
    if((num_bytes = recv(rfd, buf, sizeof(buf), 0)) <= 0) {
        if(num_bytes == 0) {
            printf("xo: socket %d exited\n", rfd);
        } else {
            perror("recv");
        }
        close(rfd);
        FD_CLR(rfd, &read_fds);
    } else {
        if (( buffers[rfd].buf_cnt + num_bytes ) >= buffers[rfd].buf_size){
             buffers[rfd].buf_size = buffers[rfd].buf_size + num_bytes + 2048;
             buffers[rfd].buf = (char *) realloc(buffers[rfd].buf, buffers[rfd].buf_size);
        }
        memcpy(buffers[rfd].buf + buffers[rfd].buf_cnt, buf, num_bytes);
        buffers[rfd].buf_cnt += num_bytes ;
    }
  
    return 0;
}

int snd(int wfd) {
    int n;
    
    if ( FD_ISSET(wfd, &read_fds)) {
        if ( wfd != sockfd ) {
            if ( (n = send(wfd, buffers[wfd].buf, buffers[wfd].buf_cnt, 0)) < 0 )
                perror("send");
            else if ( buffers[wfd].buf_cnt == n ) {
                FD_CLR(wfd, &write_fds);
                buffers[wfd].buf_cnt = 0;
            }
            else {
                memcpy(buffers[wfd].buf, buffers[wfd].buf + n, buffers[wfd].buf_cnt - n);
                buffers[wfd].buf_cnt -= n;
            }
        }
    }

    return 0;
}

int do_sendrecv() {
    int i, j;
    struct sockaddr_in their_addr;
    fd_set rfds;
    
    FD_ZERO(&rfds);
    
    memcpy(&rfds , &read_fds , sizeof(read_fds));

    if ((i = select(fdmax+1, &rfds, &write_fds, NULL, NULL)) == -1) {
        perror("select");
        exit(1);
    }

    for(j = 0; j <= fdmax && i > 0; j++) {
        if (FD_ISSET(j, &write_fds)) {
            snd(j);
            i--;
        }
        if (FD_ISSET(j, &rfds)) {
            recieve(j);
            i--;
        }
    }
        
    return 0;
}

int do_parse() {
    int i;
    
    for ( i = 1; i <= fdmax; i++) {
        if (buffers[i].buf_cnt > 0)
            FD_SET(i, &write_fds);
    }
    
    return 0;
}

int main(void) {
    
    int_socket();
    
    buffers[0].buf_size = buf_size;
    buffers[0].buf = malloc(buf_size);
    
    FD_SET(0,&read_fds);
    
    while(1) {
        do_sendrecv();
        do_parse();
    }

    return 0;
}

