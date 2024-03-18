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
    
    serverConfigurationSetMulticastJoinAddress("239.255.255.250");
    
    ServerConfiguration* serverConfiguration = serverConfigurationGet();
    
    serverCreateWithConfiguration(serverConfiguration);
    
    int exitCode = serverRun();
        
    serverCloseLogger();
    
    return exitCode;
}
