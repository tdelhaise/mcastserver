//
//  MulticastListener.h
//  mcastserver
//
//  Created by Thierry DELHAISE on 17/03/2024.
//

#ifndef MulticastListener_h
#define MulticastListener_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <sys/syslog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "Logger.h"
#include "MessageQueue.h"

#define MAX_RECEIVED_DATA 2048L

typedef struct __MulticastListenerStorage {
    int socket;
    ssize_t dataLength;
    char receivedData[MAX_RECEIVED_DATA];
    socklen_t peerAddressLength;
    struct sockaddr peerAddress;
} multicast_listener_storage_t;

void multicastListenerRun(void);
_Bool multicastListenerPrepareRun(const char* multicastJoinGroupAddress, uint16_t multicastJoinPort, message_queue_t* serverMessageQueue);
void multicastListenerShutdown(void);
void multicastListenerStop(void);

#endif /* MulticastListener_h */
