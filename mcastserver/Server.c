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
#include "Headers/MessageQueue.h"


static pthread_mutex_t serverMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct __Server {
    bool running;
    int listenFileDescriptor;
    server_configuration_t* serverConfiguration;
    bool shouldStop;
    useconds_t delay;
    message_queue_t* messageQueue;
} server_t;

static server_t currentServer = {
    .running = false,
    .listenFileDescriptor = -1,
    .serverConfiguration = NULL,
    .shouldStop = false,
    .delay = 500,
    .messageQueue = NULL
};

void serverCreateWithConfiguration(server_configuration_t* serverConfiguration) {
    if (pthread_mutex_lock(&serverMutex) == 0) {
        currentServer.serverConfiguration = serverConfiguration;
        currentServer.messageQueue = messageQueueCreate();
        pthread_mutex_unlock(&serverMutex);
        if(currentServer.messageQueue == NULL) {
            logError("serverCreateWithConfiguration: failed to create message queue !");
            exit(EXIT_FAILURE);
        }
    } else {
        logError("serverCreateWithConfiguration: failed to aquire mutex !");
        exit(EXIT_FAILURE);
    }
}

void serverOpenLogger(void) {
    openlog("mcastserver", LOG_PID | LOG_NDELAY | LOG_CONS, LOG_DAEMON);
}

void serverCloseLogger(void) {
    closelog();
}

void serverFree(void) {
    if (pthread_mutex_lock(&serverMutex) == 0) {
        if (currentServer.serverConfiguration != NULL) {
            serverConfigurationFree(currentServer.serverConfiguration);
        }
        currentServer.serverConfiguration = NULL;
        if (currentServer.messageQueue != NULL) {
            if( messageQueueDelete(currentServer.messageQueue) == false) {
                logError("serverFree: failed to delete message queue !");
            }
        }
        pthread_mutex_unlock(&serverMutex);
    } else {
        logError("serverFree: failed to acquire mutex !");
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
    
    if (multicastListenerPrepareRun(currentServer.serverConfiguration->mcastJoinGroupAddress, currentServer.serverConfiguration->mcastJoinPort, currentServer.messageQueue) == false ) {
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

void serverDispatchMessage(message_t* message) {
    
}

void serverDoJob(void) {
    // logDebug("serverDoJob: ...");
    usleep(currentServer.delay);
    
    message_t* message = messageQueuePeekMessage(currentServer.messageQueue);
    if (message != NULL) {
        serverDispatchMessage(message);
    }
}

server_exit_code_t serverRun(void) {
    
    if (currentServer.running) {
        logError("serverRun: server is allready running !");
        return serverAllreadyRunning;
    }
    
    logDebug("serverRun: will try to initialize all contexts !");
    int status = serverPrepareRun();
    if (status != serverSuccessfullyPrepared) {
        logError("serverRun: failed to prepared all contexts !");
        serverFree();
        return status;
    }
    
    logDebug("serverRun: launchMulticastListener");
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
