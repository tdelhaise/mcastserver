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
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 500
#define MAX_PORT_NUMBER 6
#define MAX_HOSTNAME 255
#define MAX_MESSAGE 255
#define DEFAULT_MULTICAST_GROUP_ADDRESS "239.255.255.250"
#define DEFAULT_MULTICAST_GROUP_PORT 12567

void resolvePeerAddress(struct sockaddr_storage* peerAddress, socklen_t peerAddressLength, char* hostname, int hostnameLength, char* service, int serviceLength) {
    if(hostnameLength < NI_MAXHOST) {
        fprintf(stderr,"resolvePeerAddress: hostnameLength parameter value less than NI_MAXHOST");
        return;
    }
    if(serviceLength < NI_MAXSERV) {
        fprintf(stderr,"resolvePeerAddress: serviceLength parameter value less than NI_MAXSERV");
        return;
    }
    
    int status = getnameinfo((struct sockaddr *) peerAddress, peerAddressLength, hostname, hostnameLength, service, serviceLength, NI_NUMERICSERV);
    if (status == 0) {
        fprintf(stdout,"resolvePeerAddress: Peer address resolved to %s:%s\n", hostname, service);
    } else {
        fprintf(stderr,"resolvePeerAddress: getnameinfo: %s\n", gai_strerror(status));
        return;
    }
}

int main(int argc, char *argv[])
{
    int              socketFileDescriptor = -1;
    int              status = 0;
    char             buffer[BUFFER_SIZE];
    size_t           len = 0;
    ssize_t          nread = 0;
    ssize_t          nsent = 0;
    char message[MAX_MESSAGE];
    struct sockaddr_in group;
    int flags = 0;
    
    memset(&group,0,sizeof(group));
    if (argc < 1) {
        fprintf(stderr, "Usage: %s message\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    memset(message,0,sizeof(message));
    
    if(strlen(message) >= MAX_MESSAGE) {
        fprintf(stderr, "message: string too long. Max = %d\n", MAX_MESSAGE);
        exit(EXIT_FAILURE);
    }

    strcpy(message, argv[1]);
    
    unsigned long messageLength = strlen(message);
    
    fprintf(stdout, "Will send [%s] %ld byte(s) to host [%s] at port [%d]\n", message, messageLength, DEFAULT_MULTICAST_GROUP_ADDRESS, DEFAULT_MULTICAST_GROUP_PORT);
    
    len = strlen(message) + 1;
            /* +1 for terminating null byte */

    if (len > BUFFER_SIZE) {
        fprintf(stderr, "Ignoring long message in argument [%s]\n", message);
        exit(EXIT_FAILURE);
    }
    
    group.sin_family = AF_INET;
    inet_pton(AF_INET,DEFAULT_MULTICAST_GROUP_ADDRESS,&group.sin_addr);
    group.sin_port = htons(DEFAULT_MULTICAST_GROUP_PORT);
    
    socketFileDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFileDescriptor == -1) {
        fprintf(stderr, "failed to get socket");
        exit(EXIT_FAILURE);
    }
    
    nsent = sendto(socketFileDescriptor, message, len, flags,(struct sockaddr*)&group,sizeof(group));
    if (nsent != len) {
        fprintf(stderr, "partial/failed write\n");
        close(socketFileDescriptor);
        exit(EXIT_FAILURE);
    }
    
    printf("Sent %zd bytes: %s\n", nsent, message);
    
    struct sockaddr_storage peerAddress;
    memset(&peerAddress, 0, sizeof(peerAddress));
    socklen_t peerAddressLength = sizeof(peerAddress);
    
    nread = recvfrom(socketFileDescriptor, buffer, BUFFER_SIZE, flags, (struct sockaddr *) &peerAddress, &peerAddressLength);
    if (nread == -1) {
        perror("recvfrom");
        close(socketFileDescriptor);
        exit(EXIT_FAILURE);
    }
    
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    host[0] = '\0';
    service[0] = '\0';
    
    resolvePeerAddress(&peerAddress, peerAddressLength, host, NI_MAXHOST, service, NI_MAXSERV);

    buffer[nread] = '\0';
    printf("Received %zd bytes: %s from %s:%s\n", nread, buffer, host, service);
    
    close(socketFileDescriptor);

    /* Success */
    exit(EXIT_SUCCESS);
}
