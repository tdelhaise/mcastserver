//
//  Server.h
//  mcastserver
//
//  Created by Thierry DELHAISE on 04/03/2024.
//

#ifndef Server_h
#define Server_h

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/syslog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>
#include "ServerConfiguration.h"



typedef struct __Server {
    bool running;
    int listenFileDescriptor;
    ServerConfiguration* serverConfiguration;
    bool shouldStop;
} Server;


typedef enum __ServerExitCode {
    successFullExit = 0,
    systemFailure,
    serverAllreadyRunning,
    serverSuccessfullyPrepared,
    failedToCreateSocket,
    failedToSetSocketListeningForSamePort,
    failedToBind,
    failedToSetSocketOptionAddMulticastMembership,
    failedToPrepareServer,
    failedToReceiveFrom,
    failedToGetNameInfo,
    failedToGetAddressInfo
} ServerExitCode;

Server* serverCopy(Server* inputServer);
void serverCreateWithConfiguration(ServerConfiguration* serverConfiguration);
void serverFree(void);
_Bool serverLaunchMulticastListener(void);
ServerExitCode serverRun(void);
void serverOpenLogger(void);
void serverCloseLogger(void);

#endif /* Server_h */
