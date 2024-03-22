//
//  MulticastListener.c
//  mcastserver
//
//  Created by Thierry DELHAISE on 17/03/2024.
//

#include "Headers/MulticastListener.h"

#define MSGBUFSIZE 512


typedef struct __MulticastListenerContext {
    _Bool prepared;
    _Bool shouldStop;
    pthread_mutex_t listenerMutex;
    int listenerSocket;
    pthread_t listenerThread;
    pthread_attr_t listenerThreadAttributes;
    size_t listenerMaxBufferSize;
    message_queue_t* messageQueue;
} multicast_listener_context_t;


static multicast_listener_context_t multicastListenerContext = {
    .prepared = false,
    .shouldStop = false,
    .listenerMutex = PTHREAD_MUTEX_INITIALIZER,
    .listenerSocket = -1,
    .listenerThreadAttributes = 0,
    .listenerMaxBufferSize = 1024,
    .messageQueue = NULL
};

_Bool __multicastListenerPrepareListenSocket(void) {
    
    //
    // create what looks like an ordinary UDP socket
    //
    multicastListenerContext.listenerSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (multicastListenerContext.listenerSocket < 0) {
        logError("__multicastListenerPrepareListenSocket: call to socket function failed ! %m");
        return false;
    }
    //
    // allow multiple sockets to use the same PORT number
    //
    u_int yes = 1;
    if (setsockopt(multicastListenerContext.listenerSocket, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes)) < 0) {
        logError("__multicastListenerPrepareListenSocket: failed to set socket option for reusing address ! %m");
        return false;
    }
    
    logInfo("__multicastListenerPrepareListenSocket: Successully prepared socket !");
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
        logError("__multicastListenerBindSocket: failed to bind ! %m");
        return false;
    }
    //
    // use setsockopt() to request that the kernel join a multicast group
    //
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multicastJoinGroupAddress);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(multicastListenerContext.listenerSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)) < 0 ) {
        logError("__multicastListenerBindSocket: failed to set socket option ! %m");
        return false;
    }

    logInfo("__multicastListenerBindSocket: Successully bind socket !");
    return true;
}

_Bool multicastListenerPrepareRun(const char* multicastJoinGroupAddress, uint16_t multicastJoinPort, message_queue_t* serverMessageQueue) {
    
    if(serverMessageQueue == NULL) {
        logError("multicastListenerPrepareRun: invalid parameter for serverMessageQueue !");
        return false;
    }
    if(multicastJoinGroupAddress == NULL || strlen(multicastJoinGroupAddress) == 0 ) {
        logError("multicastListenerPrepareRun: invalid multicastJoinGroupAddress parameter !");
        return false;
    }
    
    multicastListenerContext.messageQueue = serverMessageQueue;
    
    if(__multicastListenerPrepareListenSocket() &&
       __multicastListenerBindSocket(multicastJoinGroupAddress, multicastJoinPort) ) {
        
        multicastListenerContext.prepared = true;
        logInfo("multicastListenerPrepareRun: Successfully setup !");
        return true;
    } else {
        multicastListenerContext.prepared = false;
        logError("multicastListenerPrepareRun: failed to setup !");
        return false;
    }
}

void multicastListenerResolvePeerAddress(struct sockaddr_storage* peerAddress, socklen_t peerAddressLength, char* hostname, int hostnameLength, char* service, int serviceLength) {
    if(hostnameLength < NI_MAXHOST) {
        logError("multicastListenerResolvePeerAddress: hostnameLength parameter value less than NI_MAXHOST");
        return;
    }
    if(serviceLength < NI_MAXSERV) {
        logError("multicastListenerResolvePeerAddress: serviceLength parameter value less than NI_MAXSERV");
        return;
    }
    
    int status = getnameinfo((struct sockaddr *) peerAddress, peerAddressLength, hostname, hostnameLength, service, serviceLength, NI_NUMERICSERV);
    if (status == 0) {
        logInfo("multicastListenerResolvePeerAddress: Peer address resolved to %s:%s\n", hostname, service);
    } else {
        logError("multicastListenerResolvePeerAddress: getnameinfo: %s\n", gai_strerror(status));
        return;
    }
}

_Bool multicastListenerPrepareMessageToPost(int socket, const char* receivedData, ssize_t dataLength, struct sockaddr* peerAddress, socklen_t peerAddressLength, message_t** outputMessage) {
    if(outputMessage == NULL) {
        logError("multicastListenerPrepareMessageToPost: outputMessage parameter is NULL");
        return false;
    }
    ssize_t multicastListenerStorageSize = sizeof(multicast_listener_storage_t);
    if(dataLength > MAX_RECEIVED_DATA) {
        logError("multicastListenerPrepareMessageToPost: dataLength value is greather than the maximum size accepted: [%ld] vs [%ld] ", dataLength, MAX_RECEIVED_DATA);
        return false;
    }
    multicast_listener_storage_t* multicastListenerStorage = malloc(multicastListenerStorageSize);
    if(multicastListenerStorage == NULL) {
        logError("multicastListenerPrepareMessageToPost: failed to allocate multicast_listener_storage_t aka [%ld] bytes", multicastListenerStorageSize);
        *outputMessage = NULL;
        return false;
    }
    memset(multicastListenerStorage, 0, multicastListenerStorageSize);
    
    multicastListenerStorage->socket = socket;
    multicastListenerStorage->dataLength = dataLength;
    multicastListenerStorage->peerAddressLength = peerAddressLength;
    memcpy(&multicastListenerStorage->receivedData, receivedData, dataLength);
    memcpy(&multicastListenerStorage->peerAddress, peerAddress, peerAddressLength);
    
    ssize_t messageSize = sizeof(message_t);
    message_t* message = malloc(messageSize);
    if(message == NULL) {
        logError("multicastListenerPrepareMessageToPost: failed to allocate message_t aka [%ld] bytes", messageSize);
        free(multicastListenerStorage);
        *outputMessage = NULL;
        return false;
    }
    
    message->data = multicastListenerStorage;
    message->dataLength = multicastListenerStorageSize;
    message->next = NULL;
    message->kind = incommingPacket;
    
    *outputMessage = message;
    
    return true;
}

void* multicastListenerMain(void* unusedArgument) {
    logInfo("multicastListenerMain: started !");
    
    int flags = 0;
    
    while(multicastListenerContext.shouldStop == false) {
        char host[NI_MAXHOST];
        char service[NI_MAXSERV];
        char buffer[multicastListenerContext.listenerMaxBufferSize];
        memset(buffer, 0, sizeof(buffer));
        host[0] = '\0';
        service[0] = '\0';
        // set up destination address
        //
        struct sockaddr_storage peerAddress;
        unsigned int peerAddressSize = sizeof(peerAddress);
        memset(&peerAddress, 0, peerAddressSize);
        socklen_t peerAddressLength = peerAddressSize;
        
        ssize_t nbytes = recvfrom( multicastListenerContext.listenerSocket, buffer, multicastListenerContext.listenerMaxBufferSize, flags, (struct sockaddr *) &peerAddress, &peerAddressLength);
        if (nbytes < 0) {
            logError("multicastListenerMain: encountered an error when calling recvfrom() ! %m");
            return NULL;
        }
        buffer[nbytes] = '\0';
        
        multicastListenerResolvePeerAddress(&peerAddress, peerAddressLength, host, NI_MAXHOST, service, NI_MAXSERV);
        logInfo("multicastListenerMain: Received %zd bytes from %s:%s => [%s]\n", nbytes, host, service, buffer);
        
        message_t* outputMessage = NULL;
        if(multicastListenerPrepareMessageToPost(multicastListenerContext.listenerSocket, buffer,nbytes,(struct sockaddr *)&peerAddress,peerAddressLength,&outputMessage) == false) {
            logError("multicastListenerMain: encountered an error when preparing message to post !");
        }
        if (outputMessage != NULL && messageQueuePostMessage(multicastListenerContext.messageQueue, outputMessage) == false) {
            logError("multicastListenerMain: encountered an error when posting received message");
            // free unused message
            messageQueueDeleteMessage(outputMessage);
        }
                
        /*
        ssize_t sentBytes = sendto(multicastListenerContext.listenerSocket, buffer, nbytes, flags, (struct sockaddr *) &peerAddress, peerAddressLength);
        if (sentBytes != nbytes) {
            logError("multicastListenerMain: sendto() call failed ! %m");
        } else {
            logInfo("multicastListenerMain: Sent %ld byte(s) \n", sentBytes);
        }
        */
    }

    syslog(LOG_INFO, "multicastListenerMain: ended !");
    return NULL;
}

void multicastListenerRun(void) {
    
    if(multicastListenerContext.prepared == false) {
        logError("multicastListenerRun: multicastListenerPrepareRun() must be called before this function !");
        return;
    }
    
    int result = pthread_attr_init(&multicastListenerContext.listenerThreadAttributes);
    if ( result != 0 ) {
        const char* errorMessage = strerror(result);
        logError("multicastListenerRun: failed to initialized thread attributes !    [%d] -> %s", result, errorMessage);
        return;
    }

    result = pthread_create(&multicastListenerContext.listenerThread, &multicastListenerContext.listenerThreadAttributes, multicastListenerMain, NULL);
    if ( result != 0 ) {
        const char* errorMessage = strerror(result);
        logError("multicastListenerRun: failed to launch thread ! [%d] -> %s", result, errorMessage);
        return;
    }

    logInfo("multicastListenerRun: Thread launched !");
}

void multicastListenerWaitForTermination(void) {
    int result = pthread_join(multicastListenerContext.listenerThread, NULL);
    if( result != 0) {
        logError("multicastListenerWaitForTermination: failed to join thread ! %s", strerror(result));
    } else {
        logInfo("multicastListenerWaitForTermination: Thread terminated !");
    }
}

void multicastListenerStop(void) {
    if (multicastListenerContext.shouldStop == false) {
        pthread_mutex_lock(&multicastListenerContext.listenerMutex);
        multicastListenerContext.shouldStop = true;
        logInfo("multicastListenerStop: stop set !");
        pthread_mutex_unlock(&multicastListenerContext.listenerMutex);
    } else {
        logInfo("multicastListenerStop: stop allready set !");
    }
}

