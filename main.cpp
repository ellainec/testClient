
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    struct sockaddr_in serverAddr;
    int sfd, s, j;
    int udpSocket;
    size_t len;
    ssize_t nread;
    char buf[BUF_SIZE];
    struct hostent *he;

    /* Obtain address(es) matching host/port */

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo("127.0.0.1", "8080", &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;
        }

        close(sfd);
    }

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    //freeaddrinfo(result);           /* No longer needed */

    nread = read(sfd, buf, BUF_SIZE);

    if (nread == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    printf("port number received: %s", buf);
    if ((he=gethostbyname("127.0.0.1")) == NULL) {  /* get the host info */
        herror("gethostbyname");
        exit(1);
    }
    serverAddr.sin_family = AF_INET;      /* host byte order */
    serverAddr.sin_port = htons(atoi(buf));  /* short, network byte order */
    serverAddr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(serverAddr.sin_zero), 8);

    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        perror("udpSocket creation");
        exit(0);
    }
    memset(buf, 97, sizeof(buf));
    for(int i = 0; i < 1000; i++) {
        if(sendto(udpSocket, buf, sizeof(buf), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
            perror("write");
        }
        //printf("sent %d", i);
    }


    exit(EXIT_SUCCESS);
}