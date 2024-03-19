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
#define DEFAULT_MULTICAST_GROUP_ADDRESS "239.255.255.250"

int main(int argc, char *argv[])
{
    int              socketFileDescriptor = -1;
    int              status = 0;
    char             buf[BUF_SIZE];
    size_t           len = 0;
    ssize_t          nread = 0;
    ssize_t          nsent = 0;
    struct addrinfo  hints;
    struct addrinfo  *resultAddresses, *resultAddress;
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
    hints.ai_flags = AI_ADDRCONFIG ;
    hints.ai_protocol = 0;          /* Any protocol */

    status = getaddrinfo(hostname, portNumber, &hints, &resultAddresses);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (resultAddress = resultAddresses; resultAddress != NULL; resultAddress = resultAddress->ai_next) {
        fprintf(stdout, "socket family [%d] type [%d] proto [%d]",resultAddress->ai_family, resultAddress->ai_socktype, resultAddress->ai_protocol);
        socketFileDescriptor = socket(resultAddress->ai_family, resultAddress->ai_socktype, resultAddress->ai_protocol);
        if (socketFileDescriptor == -1) {
            fprintf(stdout, " -> Abandonned.\n");
            continue;
        }
        
        fprintf(stdout, " -> Selected.\n");
        
        if ( connect(socketFileDescriptor, resultAddress->ai_addr, resultAddress->ai_addrlen) != -1 )
            break;
        
        close(socketFileDescriptor);
        
    }

    freeaddrinfo(resultAddresses);           /* No longer needed */

    if (resultAddress == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    len = strlen(message) + 1;
            /* +1 for terminating null byte */

    if (len > BUF_SIZE) {
        fprintf(stderr, "Ignoring long message in argument [%s]\n", message);
        close(socketFileDescriptor);
        exit(EXIT_FAILURE);
    }
    
    nsent = write(socketFileDescriptor, message, len);
    if (nsent != len) {
        fprintf(stderr, "partial/failed write\n");
        close(socketFileDescriptor);
        exit(EXIT_FAILURE);
    }
    
    printf("Sent %zd bytes: %s\n", nsent, message);
    
    nread = read(socketFileDescriptor, buf, BUF_SIZE);
    if (nread == -1) {
        perror("read");
        close(socketFileDescriptor);
        exit(EXIT_FAILURE);
    }
    
    buf[nread] = '\0';
    printf("Received %zd bytes: %s\n", nread, buf);
    
    close(socketFileDescriptor);

    /* Success */
    exit(EXIT_SUCCESS);
}
