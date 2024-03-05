//
//  main.c
//  mcastserver
//
//  Created by Thierry DELHAISE on 04/03/2024.
//

#include <stdio.h>
#include "Headers/ServerConfiguration.h"
#include "Headers/Server.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    ServerConfigurationSetMulticastJoinAddress("239.255.255.250");
    
    ServerConfiguration* serverConfiguration = ServerConfigurationGet();
    
    ServerCreateWithConfiguration(serverConfiguration);
    
    int exitCode = ServerRun();
    
    return exitCode;
}
