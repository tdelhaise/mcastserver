//
//  ServerConfiguration.c
//  mcastserver
//
//  Created by Thierry DELHAISE on 04/03/2024.
//

#include "Headers/ServerConfiguration.h"

ServerConfiguration currentServerConfiguration = {
    .mcastJoinGroupAddress = "",
    .mcastJoinPort = SERVER_CONFIGURATION_DEFAULT_MCAST_PORT
};

void ServerConfigurationSetMulticastJoinPort(uint16_t portNumber) {
    assert(portNumber > 0);
    assert(portNumber < UINT16_MAX);
    currentServerConfiguration.mcastJoinPort = portNumber;
}

void ServerConfigurationSetMulticastJoinAddress(const char joinGroupAddress[MAX_JOIN_GROUP_ADDRESS_LEN]) {
    strcpy(currentServerConfiguration.mcastJoinGroupAddress,joinGroupAddress);
}

ServerConfiguration* ServerConfigurationCopy(ServerConfiguration* inputConfiguration) {
    ServerConfiguration* serverConfigurationCopy = NULL;
    
    int sizeOfStructServerConfiguration = sizeof(ServerConfiguration);
    serverConfigurationCopy = (ServerConfiguration*) malloc(sizeOfStructServerConfiguration);
    memset(serverConfigurationCopy,0,sizeOfStructServerConfiguration);
    
    strcpy(serverConfigurationCopy->mcastJoinGroupAddress, inputConfiguration->mcastJoinGroupAddress);
    serverConfigurationCopy->mcastJoinPort = inputConfiguration->mcastJoinPort;
    
    return serverConfigurationCopy;
}

ServerConfiguration* ServerConfigurationGet(void) {
    return ServerConfigurationCopy(&currentServerConfiguration);
}

void ServerConfigurationFree(ServerConfiguration* serverConfigurationCopy) {
    free(serverConfigurationCopy);
}

