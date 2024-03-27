//
//  Resolver.h
//  mcastserver
//
//  Created by Thierry DELHAISE on 23/03/2024.
//

#ifndef Resolver_h
#define Resolver_h

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

void resolverResolvePeerAddress(struct sockaddr_storage* peerAddress, socklen_t peerAddressLength, char* hostname, int hostnameLength, char* service, int serviceLength);


#endif /* Resolver_h */
