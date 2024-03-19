//
//  main.c
//  mcastserver
//
//  Created by Thierry DELHAISE on 04/03/2024.
//

#include "Headers/ServerConfiguration.h"
#include "Headers/Server.h"

int main(int argc, const char * argv[]) {
    
    serverOpenLogger();
    
    serverConfigurationSetMulticastJoinAddress(DEFAULT_MULTICAST_GROUP_ADDRESS);
    
    ServerConfiguration* serverConfiguration = serverConfigurationGet();
    
    serverCreateWithConfiguration(serverConfiguration);
    
    int exitCode = serverRun();
        
    serverCloseLogger();
    
    return exitCode;
}
