//
//  ServerConfiguration.h
//  mcastserver
//
//  Created by Thierry DELHAISE on 04/03/2024.
//

#ifndef ServerConfiguration_h
#define ServerConfiguration_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <netinet/if_ether.h>
#include "Logger.h"


#define SERVER_CONFIGURATION_DEFAULT_MCAST_PORT 12567
#define MAX_JOIN_GROUP_ADDRESS_LEN 256
#define DEFAULT_MULTICAST_GROUP_ADDRESS "239.255.255.250"

typedef struct __ServerConfiguration {
    char mcastJoinGroupAddress[MAX_JOIN_GROUP_ADDRESS_LEN];
    uint16_t mcastJoinPort;
} server_configuration_t;

void serverConfigurationSetMulticastJoinPort(uint16_t portNumber);
void serverConfigurationSetMulticastJoinAddress(const char* joinGroupAddress);
server_configuration_t* serverConfigurationGet(void);
void serverConfigurationFree(server_configuration_t* serverConfiguration);
server_configuration_t* serverConfigurationCopy(server_configuration_t* inputConfiguration);

#endif /* ServerConfiguration_h */
