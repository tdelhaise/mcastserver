//
//  Resolver.c
//  mcastserver
//
//  Created by Thierry DELHAISE on 23/03/2024.
//

#include "Headers/Resolver.h"



void resolverResolvePeerAddress(struct sockaddr_storage* peerAddress, socklen_t peerAddressLength, char* hostname, int hostnameLength, char* service, int serviceLength) {
    if(hostnameLength < NI_MAXHOST) {
        logError("resolverResolvePeerAddress: hostnameLength parameter value less than NI_MAXHOST");
        return;
    }
    if(serviceLength < NI_MAXSERV) {
        logError("resolverResolvePeerAddress: serviceLength parameter value less than NI_MAXSERV");
        return;
    }
    
    int status = getnameinfo((struct sockaddr *) peerAddress, peerAddressLength, hostname, hostnameLength, service, serviceLength, NI_NUMERICSERV);
    if (status == 0) {
        logInfo("resolverResolvePeerAddress: Peer address resolved to %s:%s\n", hostname, service);
    } else {
        logError("resolverResolvePeerAddress: getnameinfo: %s\n", gai_strerror(status));
        return;
    }
}
