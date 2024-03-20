//
//  Server.c
//  mcastserver
//
//  Created by Thierry DELHAISE on 04/03/2024.
//


#include "Headers/ServerConfiguration.h"
#include "Headers/NetworkInterface.h"
#include "Headers/Server.h"
#include "Headers/MulticastListener.h"


static pthread_mutex_t serverMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct __Server {
    bool running;
    int listenFileDescriptor;
    server_configuration_t* serverConfiguration;
    bool shouldStop;
    useconds_t delay;
} server_t;

server_t currentServer = {
    .running = false,
    .listenFileDescriptor = -1,
    .serverConfiguration = NULL,
    .shouldStop = false,
    .delay = 500
};

void serverCreateWithConfiguration(server_configuration_t* serverConfiguration) {
    if (pthread_mutex_lock(&serverMutex) == 0) {
        currentServer.serverConfiguration = serverConfiguration;
        pthread_mutex_unlock(&serverMutex);
    } else {
        
    }
}

void serverOpenLogger(void) {
    openlog("mcastserver", LOG_PID | LOG_NDELAY | LOG_CONS, LOG_DAEMON);
}

void serverCloseLogger(void) {
    closelog();
}

server_t* serverCopy(server_t* inputServer) {
    server_t* serverCopy = NULL;
    
    int sizeOfStructServer = sizeof(server_t);
    serverCopy = (server_t*) malloc(sizeOfStructServer);
    memset(serverCopy,0,sizeOfStructServer);
    
    serverCopy->running = inputServer->running;
    serverCopy->shouldStop = inputServer->shouldStop;
    serverCopy->listenFileDescriptor = inputServer->listenFileDescriptor;
    serverCopy->serverConfiguration = inputServer->serverConfiguration;
    
    return serverCopy;
}

void serverFree(void) {
    if (pthread_mutex_lock(&serverMutex) == 0) {
        if (currentServer.serverConfiguration != NULL) {
            serverConfigurationFree(currentServer.serverConfiguration);
        }
        currentServer.serverConfiguration = NULL;
        pthread_mutex_unlock(&serverMutex);
    } else {
        
    }
}

void serverStop(void) {
    if (pthread_mutex_lock(&serverMutex) == 0) {
        currentServer.shouldStop = true;
        pthread_mutex_unlock(&serverMutex);
    } else {
        
    }
}

void serverLaunchMulticastListener(void) {
    multicastListenerRun();
}

_Bool serverLaunchSender(void) {
    return false;
}


server_exit_code_t serverPrepareRun(void) {
    
    logInfo("serverPrepareRun: started !");
    
    networkInterfaceDiscover();
    
    if (multicastListenerPrepareRun(currentServer.serverConfiguration->mcastJoinGroupAddress, currentServer.serverConfiguration->mcastJoinPort) == false ) {
        logError("multicastListenerPrepareRun failed ! Exiting.");
        exit(EXIT_FAILURE);
    }
    
    logInfo("serverPrepareRun: finished !");
    return serverSuccessfullyPrepared;
}

void serverStopWorkerThreads(void) {
    logInfo("serverStopWorkerThreads: stop multicast listener ...");
    multicastListenerStop();
}

void serverDoJob(void) {
    // logDebug("serverDoJob: ...");
    usleep(currentServer.delay);
}

server_exit_code_t serverRun(void) {
    
    if (currentServer.running) {
        logError("serverRun: server is allready running !");
        return serverAllreadyRunning;
    }
    
    logInfo("serverRun: initialized all contexts !");
    int status = serverPrepareRun();
    if (status != serverSuccessfullyPrepared) {
        logError("serverRun: failed to prepared all contexts !");
        serverFree();
        return status;
    }
    
    logInfo("serverRun: launchMulticastListener");
    serverLaunchMulticastListener();
    logInfo("serverRun: Multicast Listener launched !");
    
    while(!currentServer.shouldStop) {
        serverDoJob();
    }
    
    logInfo("serverRun: stop workers thread ...");
    serverStopWorkerThreads();
    logInfo("serverRun: free ressources ...");
    serverFree();
    logInfo("serverRun: exit !");
    return successFullExit;
}
