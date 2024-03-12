//
//  Server.c
//  mcastserver
//
//  Created by Thierry DELHAISE on 04/03/2024.
//


#include "Headers/ServerConfiguration.h"
#include "Headers/NetworkInterface.h"
#include "Headers/Server.h"


#define MSGBUFSIZE 512

static pthread_mutex_t serverMutex = PTHREAD_MUTEX_INITIALIZER;


Server currentServer = {
    .running = false,
    .listenFileDescriptor = -1,
    .serverConfiguration = NULL,
    .shouldStop = false
};

void ServerCreateWithConfiguration(ServerConfiguration* serverConfiguration) {
    if (pthread_mutex_lock(&serverMutex) == 0) {
        currentServer.serverConfiguration = serverConfiguration;
        pthread_mutex_unlock(&serverMutex);
    } else {
        
    }
}


Server* ServerCopy(Server* inputServer) {
    Server* serverCopy = NULL;
    
    int sizeOfStructServer = sizeof(Server);
    serverCopy = (Server*) malloc(sizeOfStructServer);
    memset(serverCopy,0,sizeOfStructServer);
    
    serverCopy->running = inputServer->running;
    serverCopy->shouldStop = inputServer->shouldStop;
    serverCopy->listenFileDescriptor = inputServer->listenFileDescriptor;
    serverCopy->serverConfiguration = inputServer->serverConfiguration;
    
    return serverCopy;
}

void ServerFree(void) {
    if (pthread_mutex_lock(&serverMutex) == 0) {
        if (currentServer.serverConfiguration != NULL) {
            ServerConfigurationFree(currentServer.serverConfiguration);
        }
        currentServer.serverConfiguration = NULL;
        pthread_mutex_unlock(&serverMutex);
    } else {
        
    }
}

void ServerShouldStop(void) {
    if (pthread_mutex_unlock(&serverMutex) == 0) {
        currentServer.shouldStop = true;
        pthread_mutex_unlock(&serverMutex);
    } else {
        
    }
}

_Bool __ServerCreateListenFileDescriptor(void) {
    
    //
    // create what looks like an ordinary UDP socket
    //
    currentServer.listenFileDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (currentServer.listenFileDescriptor < 0) {
        perror("socket");
        return false;
    }
    //
    // allow multiple sockets to use the same PORT number
    //
    u_int yes = 1;
    if (setsockopt(currentServer.listenFileDescriptor, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes)) < 0) {
        perror("Reusing ADDR failed");
        return false;
    }
    
    return true;
}

ServerExitCode ServerPrepareRun(void) {
    
    discoverNetworkInterface();
    
    if (pthread_mutex_lock(&serverMutex) == 0) {
        if(__ServerCreateListenFileDescriptor() == false) {
            pthread_mutex_unlock(&serverMutex);
            ServerShouldStop();
            return failedToCreateSocket;
        }

        // set up destination address
        //
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender
        addr.sin_port = htons(currentServer.serverConfiguration->mcastJoinPort);
        //
        // bind to receive address
        //
        if (bind(currentServer.listenFileDescriptor, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
            perror("bind");
            pthread_mutex_unlock(&serverMutex);
            ServerShouldStop();
            return failedToBind;
        }
        //
        // use setsockopt() to request that the kernel join a multicast group
        //
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(currentServer.serverConfiguration->mcastJoinGroupAddress);
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if (setsockopt(currentServer.listenFileDescriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)) < 0 ) {
            perror("setsockopt");
            pthread_mutex_unlock(&serverMutex);
            ServerShouldStop();
            return failedToSetSocketOptionAddMulticastMembership;
        }
        pthread_mutex_unlock(&serverMutex);
        return serverSuccessfullyPrepared;
    } else {
        return failedToPrepareServer;
    }
}

ServerExitCode ServerRun(void) {
    if (currentServer.running) {
        return serverAllreadyRunning;
    }
    
    int status = ServerPrepareRun();
    if (status != serverSuccessfullyPrepared) {
        ServerFree();
        return status;
    }

    while(currentServer.shouldStop == false) {
        char host[NI_MAXHOST], service[NI_MAXSERV];
        char msgbuf[MSGBUFSIZE];
        memset(msgbuf, 0, sizeof(msgbuf));
        // set up destination address
        //
        struct sockaddr_storage peerAddress;
        memset(&peerAddress, 0, sizeof(peerAddress));
        socklen_t peerAddressLength = sizeof(peerAddress);
        
        ssize_t nbytes = recvfrom( currentServer.listenFileDescriptor, msgbuf, MSGBUFSIZE, 0, (struct sockaddr *) &peerAddress, &peerAddressLength);
        if (nbytes < 0) {
            perror("recvfrom");
            return failedToReceiveFrom;
        }
        msgbuf[nbytes] = '\0';
        
        int status = getnameinfo((struct sockaddr *) &peerAddress, peerAddressLength, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
        if (status == 0) {
            printf("Received %zd bytes from %s:%s => [%s]\n", nbytes, host, service, msgbuf);
        } else {
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(status));
            return failedToGetNameInfo;
        }
                
        ssize_t sentBytes = sendto(currentServer.listenFileDescriptor, msgbuf, nbytes, 0, (struct sockaddr *) &peerAddress, peerAddressLength);
        if (sentBytes != nbytes) {
            perror("sendto");
            fprintf(stderr, "Error sending response\n");
        } else {
            fprintf(stdout, "Sent %ld byte(s) \n", sentBytes);
        }
    }
    
    ServerFree();
    return successFullExit;
}
