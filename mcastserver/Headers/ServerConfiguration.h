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


#define SERVER_CONFIGURATION_DEFAULT_MCAST_PORT 12567
#define MAX_JOIN_GROUP_ADDRESS_LEN 256

struct __ServerConfiguration {
    char mcastJoinGroupAddress[MAX_JOIN_GROUP_ADDRESS_LEN];
    uint16_t mcastJoinPort;
};

typedef struct __ServerConfiguration ServerConfiguration;

void ServerConfigurationSetMulticastJoinPort(uint16_t portNumber);
void ServerConfigurationSetMulticastJoinAddress(const char* joinGroupAddress);
ServerConfiguration* ServerConfigurationGet(void);
void ServerConfigurationFree(ServerConfiguration* serverConfiguration);
ServerConfiguration* ServerConfigurationCopy(ServerConfiguration* inputConfiguration);

#endif /* ServerConfiguration_h */
