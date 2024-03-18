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

void serverConfigurationSetMulticastJoinPort(uint16_t portNumber) {
    assert(portNumber > 0);
    assert(portNumber < UINT16_MAX);
    currentServerConfiguration.mcastJoinPort = portNumber;
}

void serverConfigurationSetMulticastJoinAddress(const char* joinGroupAddress) {
    if (joinGroupAddress != NULL) {
        assert(strlen(joinGroupAddress) <= MAX_JOIN_GROUP_ADDRESS_LEN);
        strcpy(currentServerConfiguration.mcastJoinGroupAddress,joinGroupAddress);
    }
}

ServerConfiguration* serverConfigurationCopy(ServerConfiguration* inputConfiguration) {
    ServerConfiguration* serverConfigurationCopy = NULL;
    
    int sizeOfStructServerConfiguration = sizeof(ServerConfiguration);
    serverConfigurationCopy = (ServerConfiguration*) malloc(sizeOfStructServerConfiguration);
    memset(serverConfigurationCopy,0,sizeOfStructServerConfiguration);
    
    strcpy(serverConfigurationCopy->mcastJoinGroupAddress, inputConfiguration->mcastJoinGroupAddress);
    serverConfigurationCopy->mcastJoinPort = inputConfiguration->mcastJoinPort;
    
    return serverConfigurationCopy;
}

ServerConfiguration* serverConfigurationGet(void) {
    return serverConfigurationCopy(&currentServerConfiguration);
}

void serverConfigurationFree(ServerConfiguration* serverConfigurationCopy) {
    free(serverConfigurationCopy);
}

