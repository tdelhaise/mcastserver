//
//  Sender.c
//  mcastserver
//
//  Created by Thierry DELHAISE on 24/03/2024.
//

#include "Headers/Sender.h"


typedef struct __SenderContext {
    _Bool prepared;
    _Bool shouldStop;
    pthread_mutex_t senderMutex;
    pthread_t senderThread;
    pthread_attr_t senderThreadAttributes;
    size_t senderMaxBufferSize;
    message_queue_t* messageQueue;
    useconds_t delay;
} sender_context_t;

static sender_context_t senderContext = {
    .prepared = false,
    .shouldStop = false,
    .senderMutex = PTHREAD_MUTEX_INITIALIZER,
    .senderThreadAttributes = 0,
    .senderMaxBufferSize = 1024,
    .messageQueue = NULL,
    .delay = 1000
};

void senderDispatchMessage(message_t* message) {
    usleep(senderContext.delay);
}

void senderDoJob(void) {
    message_t* incommingMessage = messageQueuePeekMessage(senderContext.messageQueue);
    if (incommingMessage != NULL) {
        senderDispatchMessage(incommingMessage);
    }

}

void* senderMain(void* unusedArgument) {
    logInfo("senderMain: started !");
    
    while(senderContext.shouldStop == false) {
        senderDoJob();
    }
    
    logInfo("senderMain: ended !");
    return NULL;
}

_Bool senderRun(void) {
    logInfo("senderRun: started !");
    
    if(senderContext.prepared == false) {
        logError("senderRun: senderPrepareRun() must be called before this function !");
        return false;
    }
    
    int result = pthread_attr_init(&senderContext.senderThreadAttributes);
    if ( result != 0 ) {
        const char* errorMessage = strerror(result);
        logError("senderRun: failed to initialized thread attributes ! [%d] -> %s", result, errorMessage);
        return false;
    }

    result = pthread_create(&senderContext.senderThread, &senderContext.senderThreadAttributes, senderMain, NULL);
    if ( result != 0 ) {
        const char* errorMessage = strerror(result);
        logError("senderRun: failed to launch thread ! [%d] -> %s", result, errorMessage);
        return false;
    }

    logInfo("senderRun: Thread launched !");
    return true;
}

_Bool senderPrepareRun(message_queue_t* senderMessageQueue) {
    logInfo("senderPrepareRun: started !");
    if(senderMessageQueue == NULL) {
        logError("senderPrepareRun: invalid parameter for serverMessageQueue !");
        return false;
    }
    
    senderContext.messageQueue = senderMessageQueue;
    
    // set sender prepared !
    senderContext.prepared = true;
    logInfo("senderPrepareRun: sender succesffully prepared !");
    return true;
}

void senderShutdown(void) {
    logInfo("senderShutdown: started !");
    
}

void senderStop(void) {
    logInfo("senderStop: started !");
    if (senderContext.shouldStop == false) {
        pthread_mutex_lock(&senderContext.senderMutex);
        senderContext.shouldStop = true;
        logInfo("senderStop: stop set !");
        pthread_mutex_unlock(&senderContext.senderMutex);
    } else {
        logInfo("senderStop: stop allready set !");
    }
}

void senderWaitForTermination(void) {
    int result = pthread_join(senderContext.senderThread, NULL);
    if( result != 0) {
        logError("senderWaitForTermination: failed to join thread ! %s", strerror(result));
    } else {
        logInfo("senderWaitForTermination: Thread terminated !");
    }
}
