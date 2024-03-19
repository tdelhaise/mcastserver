//
//  MulticastListener.c
//  mcastserver
//
//  Created by Thierry DELHAISE on 17/03/2024.
//

#include "MulticastListener.h"


#define MSGBUFSIZE 512


typedef struct __MulticastListenerContext {
    _Bool prepared;
    _Bool shouldStop;
    pthread_mutex_t listenerMutex;
    int listenerSocket;
    pthread_t listenerThread;
    pthread_attr_t listenerThreadAttributes;
    uint32_t listenerMaxBufferSize;
} MulticastListenerContext;


static MulticastListenerContext multicastListenerContext = {
    .prepared = false,
    .shouldStop = false,
    .listenerMutex = PTHREAD_MUTEX_INITIALIZER,
    .listenerSocket = -1,
    .listenerThreadAttributes = 0,
    .listenerMaxBufferSize = 1024
};

_Bool __multicastListenerPrepareListenSocket(void) {
    
    //
    // create what looks like an ordinary UDP socket
    //
    multicastListenerContext.listenerSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (multicastListenerContext.listenerSocket < 0) {
        syslog(LOG_ERR, "__multicastListenerPrepareListenSocket: call to socket function failed ! %m");
        return false;
    }
    //
    // allow multiple sockets to use the same PORT number
    //
    u_int yes = 1;
    if (setsockopt(multicastListenerContext.listenerSocket, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes)) < 0) {
        syslog(LOG_ERR, "__multicastListenerPrepareListenSocket: failed to set socket option for reusing address ! %m");
        return false;
    }
    
    syslog(LOG_INFO, "__multicastListenerPrepareListenSocket: Successully prepared socket !");
    return true;
}

_Bool __multicastListenerBindSocket(const char* multicastJoinGroupAddress, uint16_t multicastJoinPort) {
    // set up destination address
    //
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender
    addr.sin_port = htons(multicastJoinPort);
    //
    // bind to receive address
    //
    if (bind(multicastListenerContext.listenerSocket, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        syslog(LOG_ERR, "__multicastListenerBindSocket: failed to bind ! %m");
        return false;
    }
    //
    // use setsockopt() to request that the kernel join a multicast group
    //
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multicastJoinGroupAddress);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(multicastListenerContext.listenerSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)) < 0 ) {
        syslog(LOG_ERR, "__multicastListenerBindSocket: failed to set socket option ! %m");
        return false;
    }

    syslog(LOG_INFO, "__multicastListenerBindSocket: Successully bind socket !");
    return true;
}

_Bool multicastListenerPrepareRun(const char* multicastJoinGroupAddress, uint16_t multicastJoinPort) {
    
    if(__multicastListenerPrepareListenSocket() &&
       __multicastListenerBindSocket(multicastJoinGroupAddress, multicastJoinPort)) {
        multicastListenerContext.prepared = true;
        return true;
    }
        
    multicastListenerContext.prepared = false;
    syslog(LOG_INFO, "multicastListenerPrepareRun: Successully setup !");
    return false;
}

void* multicastListenerMain(void* unusedArgument) {
    syslog(LOG_INFO, "multicastListenerMain: started !");
    
    int flags = 0;
    
    while(multicastListenerContext.shouldStop == false) {
        char host[NI_MAXHOST], service[NI_MAXSERV];
        char msgbuf[multicastListenerContext.listenerMaxBufferSize];
        memset(msgbuf, 0, sizeof(msgbuf));
        // set up destination address
        //
        struct sockaddr_storage peerAddress;
        memset(&peerAddress, 0, sizeof(peerAddress));
        socklen_t peerAddressLength = sizeof(peerAddress);
        
        ssize_t nbytes = recvfrom( multicastListenerContext.listenerSocket, msgbuf, multicastListenerContext.listenerMaxBufferSize, flags, (struct sockaddr *) &peerAddress, &peerAddressLength);
        if (nbytes < 0) {
            syslog(LOG_ERR,"multicastListenerMain: encountered an error when calling recvfrom() ! %m");
            return NULL;
        }
        msgbuf[nbytes] = '\0';
        
        int status = getnameinfo((struct sockaddr *) &peerAddress, peerAddressLength, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
        if (status == 0) {
            syslog(LOG_INFO,"multicastListenerMain: Received %zd bytes from %s:%s => [%s]\n", nbytes, host, service, msgbuf);
        } else {
            syslog(LOG_ERR,"multicastListenerMain: getnameinfo: %s\n", gai_strerror(status));
            return NULL;
        }
                
        ssize_t sentBytes = sendto(multicastListenerContext.listenerSocket, msgbuf, nbytes, flags, (struct sockaddr *) &peerAddress, peerAddressLength);
        if (sentBytes != nbytes) {
            syslog(LOG_ERR, "multicastListenerMain: sendto() call failed ! %m");
        } else {
            syslog(LOG_INFO, "multicastListenerMain: Sent %ld byte(s) \n", sentBytes);
        }
    }

    syslog(LOG_INFO, "multicastListenerMain: ended !");
    return NULL;
}

void multicastListenerRun(void) {
    
    if(multicastListenerContext.prepared == false) {
        syslog(LOG_ERR, "multicastListenerRun: multicastListenerPrepareRun() must be called before this function !");
        return;
    }
    
    int result = pthread_attr_init(&multicastListenerContext.listenerThreadAttributes);
    if ( result != 0 ) {
        const char* errorMessage = strerror(result);
        syslog(LOG_ERR, "multicastListenerRun: failed to initialized thread attributes !    [%d] -> %s", result, errorMessage);
        return;
    }

    result = pthread_create(&multicastListenerContext.listenerThread, &multicastListenerContext.listenerThreadAttributes, multicastListenerMain, NULL);
    if ( result != 0 ) {
        const char* errorMessage = strerror(result);
        syslog(LOG_ERR, "multicastListenerRun: failed to launch thread ! [%d] -> %s", result, errorMessage);
        return;
    }

    syslog(LOG_INFO, "multicastListenerRun: Thread launched !");
}

void multicastListenerWaitForTermination(void) {
    if(pthread_join(multicastListenerContext.listenerThread, NULL) != 0) {
        syslog(LOG_ERR, "MulticastListenerWaitForTermination: failed to join thread !");
    }
    syslog(LOG_INFO, "MulticastListenerWaitForTermination: Thread terminated !");
}

