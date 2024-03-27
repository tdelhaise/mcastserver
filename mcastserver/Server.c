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
#include "Headers/Sender.h"
#include "Headers/MessageQueue.h"


static pthread_mutex_t serverMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct __Server {
    bool running;
    int listenFileDescriptor;
    server_configuration_t* serverConfiguration;
    bool shouldStop;
    useconds_t delay;
    message_queue_t* incommingQueue;
    message_queue_t* outgoingQueue;
} server_t;

static server_t currentServer = {
    .running = false,
    .listenFileDescriptor = -1,
    .serverConfiguration = NULL,
    .shouldStop = false,
    .delay = 1000,
    .incommingQueue = NULL,
    .outgoingQueue = NULL,
};

void serverCreateWithConfiguration(server_configuration_t* serverConfiguration) {
    if (pthread_mutex_lock(&serverMutex) == 0) {
        currentServer.serverConfiguration = serverConfiguration;
        currentServer.incommingQueue = messageQueueCreate();
        currentServer.outgoingQueue = messageQueueCreate();
        pthread_mutex_unlock(&serverMutex);
        if(currentServer.incommingQueue == NULL || currentServer.outgoingQueue == NULL) {
            logError("serverCreateWithConfiguration: failed to create message queues !");
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
        if (currentServer.incommingQueue != NULL) {
            if( messageQueueDelete(currentServer.incommingQueue) == false) {
                logError("serverFree: failed to delete message queue !");
            }
        }
        if (currentServer.outgoingQueue != NULL) {
            if( messageQueueDelete(currentServer.outgoingQueue) == false) {
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

_Bool serverLaunchMulticastListener(void) {
    logInfo("serverLaunchMulticastListener: started !");
    return multicastListenerRun();
}

_Bool serverLaunchSender(void) {
    logInfo("serverLaunchSender: started !");
    return senderRun();
}


server_exit_code_t serverPrepareRun(void) {
    
    logInfo("serverPrepareRun: started !");
    
    networkInterfaceDiscover();
    
    if (multicastListenerPrepareRun(currentServer.serverConfiguration->mcastJoinGroupAddress, currentServer.serverConfiguration->mcastJoinPort, currentServer.incommingQueue) == false ) {
        logError("serverPrepareRun: multicast listener preparation failed ! Exiting.");
        exit(EXIT_FAILURE);
    }
    
    if(senderPrepareRun(currentServer.outgoingQueue) == false ) {
        logError("serverPrepareRun: sender preparation failed ! Exiting.");
        exit(EXIT_FAILURE);

    }
    
    logInfo("serverPrepareRun: finished !");
    return serverSuccessfullyPrepared;
}

void serverStopWorkerThreads(void) {
    logInfo("serverStopWorkerThreads: stop multicast listener ...");
    multicastListenerStop();
    logInfo("serverStopWorkerThreads: stop sender ...");
    senderStop();
}

void serverDecodeIncommingPacket(message_t* message) {
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    host[0] = '\0';
    service[0] = '\0';

    multicast_listener_storage_t* multicastListenerStorage = (multicast_listener_storage_t*) message->data;
    logInfo("serverDecodeIncommingPacket: received an incommingPacket of size [%ld] bytes of data.", multicastListenerStorage->dataLength);
    
    resolverResolvePeerAddress(&multicastListenerStorage->peerAddress, multicastListenerStorage->peerAddressLength, host, NI_MAXHOST, service, NI_MAXSERV);
    logInfo("serverDecodeIncommingPacket: received an incommingPacket from %s:%s", host, service);
}

void serverDispatchMessage(message_t* message) {
    logInfo("serverDispatchMessage: received a new message kind [%d] of type [%d] with [%ld] bytes of data.", message->kind, message->type, message->dataLength);
    switch (message->kind) {
        case incommingPacket: {
            logInfo("serverDispatchMessage: handle [incommingPacket] message.");
            serverDecodeIncommingPacket(message);
            break;
        }
        default: {
            logInfo("serverDispatchMessage: drop unmanaged message.");
            messageQueueDeleteMessage(message);
            break;
        }
    }
}

void serverDoJob(void) {
    // logDebug("serverDoJob: ...");
    usleep(currentServer.delay);
    
    message_t* incommingMessage = messageQueuePeekMessage(currentServer.incommingQueue);
    if (incommingMessage != NULL) {
        serverDispatchMessage(incommingMessage);
    }
}

_Bool serverLaunchWorkerThreads(void) {
    logDebug("serverLaunchWorkerThreads: launch Multicast Listener");
    if (serverLaunchMulticastListener() == false) {
        logInfo("serverLaunchWorkerThreads: failed to launch Multicast Listener !");
        return false;
    } else {
        logInfo("serverLaunchWorkerThreads: Multicast Listener launched !");
    }
    logDebug("serverLaunchWorkerThreads: launch Sender");
    if (serverLaunchSender() == false) {
        logInfo("serverLaunchWorkerThreads: failed to launch Sender !");
        return false;
    } else {
        logInfo("serverLaunchWorkerThreads: Sender launched !");
    }
    return true;
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
    
    logDebug("serverRun: launch workers threads ...");
    if (serverLaunchWorkerThreads() == false) {
        serverFree();
        return failedToLaunchWorkerThreads;
    }

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
