//
//  main.c
//  mcastclient
//
//  Created by Thierry DELHAISE on 05/03/2024.
//

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_SIZE 500
#define MAX_PORT_NUMBER 6
#define MAX_HOSTNAME 255
#define MAX_MESSAGE 255

int main(int argc, char *argv[])
{
    int              socketFileDescriptor = -1;
    int              status = 0;
    char             buf[BUF_SIZE];
    size_t           len;
    struct addrinfo  hints;
    struct addrinfo  *result, *rp;
    char portNumber[MAX_PORT_NUMBER];
    uint16_t portAsInteger = -1;
    char hostname[MAX_HOSTNAME];
    char message[MAX_MESSAGE];

    if (argc < 3) {
        fprintf(stderr, "Usage: %s hostname port message\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    memset(portNumber,0,sizeof(portNumber));
    memset(hostname,0,sizeof(hostname));
    memset(message,0,sizeof(message));
    
    if(strlen(portNumber) >= MAX_PORT_NUMBER) {
        fprintf(stderr, "port: string too long. Max = %d\n", MAX_PORT_NUMBER);
        exit(EXIT_FAILURE);
    }

    if(strlen(hostname) >= MAX_HOSTNAME) {
        fprintf(stderr, "hostname: string too long. Max = %d\n", MAX_HOSTNAME);
        exit(EXIT_FAILURE);
    }

    if(strlen(message) >= MAX_MESSAGE) {
        fprintf(stderr, "message: string too long. Max = %d\n", MAX_MESSAGE);
        exit(EXIT_FAILURE);
    }

    strcpy(hostname,argv[1]);
    strcpy(portNumber, argv[2]);
    strcpy(message, argv[3]);
    
    unsigned long messageLength = strlen(message);
    
    fprintf(stdout, "Will send [%s] %ld byte(s) to host [%s] at port [%s]\n", message, messageLength, hostname, portNumber);
    
    portAsInteger = atoi(portNumber);

    /* Obtain address(es) matching host/port. */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    status = getaddrinfo(hostname, portNumber, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        fprintf(stdout, "socket family [%d] type [%d] proto [%d]\n",rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        socketFileDescriptor = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (socketFileDescriptor == -1)
            continue;
        
        // set up destination address
        //
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender
        addr.sin_port = htons(portAsInteger);
        //
        // bind to receive address
        //
        if (bind(socketFileDescriptor, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
            perror("bind");
            close(socketFileDescriptor);
            continue;
        }
        //
        // use setsockopt() to request that the kernel join a multicast group
        //
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(hostname);
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if (setsockopt(socketFileDescriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)) < 0 ) {
            perror("setsockopt");
            close(socketFileDescriptor);
            continue;
        }
        /* Success */
        break;
    }

    freeaddrinfo(result);           /* No longer needed */

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    /* Send remaining command-line arguments as separate
       datagrams, and read responses from server. */
    for (size_t j = 3; j < argc; j++) {
        len = strlen(message) + 1;
                /* +1 for terminating null byte */

        if (len > BUF_SIZE) {
            fprintf(stderr, "Ignoring long message in argument %zu\n", j);
            continue;
        }
        
        // set up destination address
        //
        struct sockaddr_storage peerAddress;
        memset(&peerAddress, 0, sizeof(peerAddress));
        socklen_t peerAddressLength = sizeof(peerAddress);
        
        ssize_t sentBytes = sendto(socketFileDescriptor, message, len, 0, (struct sockaddr *) &peerAddress, peerAddressLength);
        if (sentBytes != len) {
            perror("sendto");
            fprintf(stderr, "Error sending message\n");
            exit(EXIT_FAILURE);
        } else {
            fprintf(stdout, "Sent %ld byte(s) \n", sentBytes);
        }

        /*
        if (write(socketFileDescriptor, message, len) != len) {
            fprintf(stderr, "partial/failed write\n");
            exit(EXIT_FAILURE);
        }
        
        nread = read(socketFileDescriptor, buf, BUF_SIZE);
        if (nread == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        */
        
        ssize_t nbytes = recvfrom( socketFileDescriptor, buf, BUF_SIZE, 0, (struct sockaddr *) &peerAddress, &peerAddressLength);
        if (nbytes < 0) {
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }
        buf[nbytes] = '\0';


        printf("Received %zd bytes: %s\n", nbytes, buf);
    }
    
    close(socketFileDescriptor);

    /* Success */
    exit(EXIT_SUCCESS);
}
