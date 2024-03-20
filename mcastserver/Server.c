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
    
    syslog(LOG_INFO,"serverPrepareRun: started !");
    
    networkInterfaceDiscover();
    
    if (multicastListenerPrepareRun(currentServer.serverConfiguration->mcastJoinGroupAddress, currentServer.serverConfiguration->mcastJoinPort) == false ) {
        syslog(LOG_ERR,"multicastListenerPrepareRun failed ! Exiting.");
        exit(EXIT_FAILURE);
    }
    
    syslog(LOG_INFO,"serverPrepareRun: finished !");
    return serverSuccessfullyPrepared;
}

void serverStopWorkerThreads(void) {
    syslog(LOG_INFO,"serverStopWorkerThreads: stop multicast listener ...");
    multicastListenerStop();
}

void serverDoJob(void) {
    syslog(LOG_INFO,"serverDoJob: ...");
    usleep(currentServer.delay);
}

server_exit_code_t serverRun(void) {
    
    
    if (currentServer.running) {
        syslog(LOG_ERR,"serverRun: server is allready running !");
        return serverAllreadyRunning;
    }
    
    syslog(LOG_INFO,"serverRun: initialized all contexts !");
    int status = serverPrepareRun();
    if (status != serverSuccessfullyPrepared) {
        syslog(LOG_ERR,"serverRun: failed to prepared all contexts !");
        serverFree();
        return status;
    }
    
    syslog(LOG_INFO,"serverRun: launchMulticastListener");
    serverLaunchMulticastListener();
    syslog(LOG_INFO,"serverRun: Multicast Listener launched !");
    
    while(!currentServer.shouldStop) {
        serverDoJob();
    }
    
    syslog(LOG_INFO,"serverRun: stop workers thread ...");
    serverStopWorkerThreads();
    syslog(LOG_INFO,"serverRun: free ressources ...");
    serverFree();
    syslog(LOG_INFO,"serverRun: exit !");
    return successFullExit;
}
