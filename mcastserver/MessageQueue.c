//
//  MessageQueue.c
//  mcastserver
//
//  Created by Thierry DELHAISE on 20/03/2024.
//

#include "MessageQueue.h"


message_queue_t* messageQueueCreate(void) {
    
    message_queue_t* newMessageQueue = malloc(sizeof(message_queue_t));
    if(newMessageQueue == NULL) {
        logError("messageQueueCreate: Failed to allocatate room for new message queue !");
        return NULL;
    }
    
    if( pthread_mutex_init(&newMessageQueue->mutex, NULL) != 0) {
        logError("messageQueueCreate: Failed to initialize mutex ! %m");
        free(newMessageQueue);
        return NULL;
    }
    
    newMessageQueue->head = NULL;
    newMessageQueue->end = NULL;
    newMessageQueue->messageCount = 0;
    newMessageQueue->opened = true;
    
    return newMessageQueue;
}

_Bool messageQueueCheckMessageConsistancy(message_t* message) {
    if (message == NULL) {
        logError("messageQueueCheckMessageConsistancy: invalid parameter for message: NULL pointer provided !");
        return false;
    }
    
    if (message->kind == undefined) {
        logError("messageQueueCheckMessageConsistancy: can't insert/delete a message with undefined type !");
        return false;
    }
    
    if (message->dataLength <= 0) {
        logError("messageQueueCheckMessageConsistancy: can't insert/delete a message with a data length <= 0 !");
        return false;
    }

    if (message->data == NULL) {
        logError("messageQueueCheckMessageConsistancy: can't insert/delete a message with a data buffer set to NULL !");
        return false;
    }
    
    if (message->next != NULL) {
        logError("messageQueueCheckMessageConsistancy: can't insert/delete a message with an unitialized next field != NULL. This field must be NULL when calling this function !");
        return false;
    }
    return true;
}

_Bool messageQueueCheckQueueConsistancy(message_queue_t* queue) {
    if (queue == NULL) {
        logError("messageQueueCheckQueueConsistancy: invalid parameter for queue: NULL pointer provided !");
        return false;
    }
        
    if (queue->opened == false) {
        logError("messageQueueCheckQueueConsistancy: invalid queue state ! Queue must be opened !");
        return false;
    }
    
    if (queue->head == NULL && queue->messageCount != 0) {
        logError("messageQueueCheckQueueConsistancy: Queue inconsitancy ! head field can't be NULL and message count with a value different of zero !");
        return false;
    }
    
    if (queue->head == NULL && queue->end != NULL) {
        logError("messageQueueCheckQueueConsistancy: Queue inconsitancy ! head field can't be NULL and end field with a value not equal to NULL !");
        return false;
    }
    
    if( queue->head != NULL && queue->head == queue->end && queue->messageCount != 1) {
        logError("messageQueueCheckQueueConsistancy: Queue inconsitancy ! head field can't be equal to end field with a messageCount != 1 !");
        return false;
    }
    
    if (queue->end != NULL && queue->end->next != NULL) {
        logError("messageQueueCheckQueueConsistancy: Queue inconsitancy ! next field of end message must be NULL !");
        return false;
    }

    return true;
}

_Bool messageQueueDelete(message_queue_t* queue) {
    message_t* message = NULL;
    if(messageQueueCheckQueueConsistancy(queue) == false) {
        return false;
    }
    if(pthread_mutex_lock(&queue->mutex) == 0) {
        queue->opened = false;
        message = queue->head;
        while( message != NULL ) {
            message_t* nextMessage = message->next;
            if( messageQueueDeleteMessage(message) == false ) {
                logWarning("messageQueueDelete: failed to delete message !");
            }
            message = nextMessage;
        }
        queue->end = NULL;
        queue->head = NULL;
        queue->messageCount = 0;
        pthread_mutex_unlock(&queue->mutex);
        if(pthread_mutex_destroy(&queue->mutex) != 0) {
            logError("messageQueueDelete: unable to destroy mutex of the queue ! %m");
        }
        free(queue);
    } else {
        logError("messageQueueDelete: unable to aquire mutex of the queue ! %m");
        return false;
    }
    return true;
}

message_t* messageQueueCreateMessage(void* data, uint32_t dataLength, message_kind_t kind, message_structure_t structure) {
    message_t* newMessage = NULL;
    
    if(data == NULL) {
        logError("messageQueueCreateMessage: data parameter is null !");
        return NULL;
    }
    
    if(dataLength == 0) {
        logError("messageQueueCreateMessage: dataLength parameter is equal to zero !");
        return NULL;
    }

    if(kind == undefined) {
        logError("messageQueueCreateMessage: kind parameter is undefined !");
        return NULL;
    }

    if(structure == unknown) {
        logError("messageQueueCreateMessage: structure parameter is unknown !");
        return NULL;
    }
    
    newMessage = malloc(sizeof(message_t));
    if(newMessage == NULL) {
        logError("messageQueueCreateMessage: failed to allocate new message !");
        return NULL;
    }
    
    newMessage->data = data;
    newMessage->dataLength = dataLength;
    newMessage->kind = kind;
    newMessage->structure = structure;
    newMessage->next = NULL;
    
    return newMessage;
}

message_t* messageQueuePeekMessage(message_queue_t* queue) {
    message_t* peekMessage = NULL;
    
    if(messageQueueCheckQueueConsistancy(queue) == false) {
        return NULL;
    }
    
    if(queue->head == NULL) {
        return NULL;
    } else {
        peekMessage = queue->head;
        if(pthread_mutex_lock(&queue->mutex) == 0) {
            queue->head = queue->head->next;
            queue->messageCount = queue->messageCount - 1;
            peekMessage->next = NULL;
            if(queue->head == NULL) {
                queue->end = NULL;
            }
            pthread_mutex_unlock(&queue->mutex);
        } else {
            logError("messageQueuePeekMessage: unable to aquire mutex of the queue ! %m");
            return NULL;
        }
    }

    return peekMessage;
}

_Bool messageQueueDeleteMessage(message_t* messageToDelete) {
    if(messageQueueCheckMessageConsistancy(messageToDelete) == false) {
        return false;
    }
    messageToDelete->next = NULL;
    
    if(messageToDelete->data != NULL) {
        free(messageToDelete->data);
        messageToDelete->data = NULL;
    } else {
        logError("messageQueueDeleteMessage: data pointer is NULL where it should not !");
    }
    
    free(messageToDelete);
    messageToDelete = NULL;
    return true;
}

_Bool messageQueuePostMessage(message_queue_t* queue, message_t* message) {
    
    if(messageQueueCheckMessageConsistancy(message) == false) {
        return false;
    }
    
    if(messageQueueCheckQueueConsistancy(queue) == false) {
        return false;
    }
    
    if (queue->end == NULL) {
        // there is actually no message in the queue !
        if(pthread_mutex_lock(&queue->mutex) == 0) {
            queue->end = message;
            queue->head = message;
            queue->messageCount = 1;
            pthread_mutex_unlock(&queue->mutex);
        } else {
            logError("messageQueuePostMessage: unable to aquire mutex of the queue ! %m");
            return false;
        }
    } else {
        // there is message in queue so we will insert the new one
        // starting at the end of the queue
        if(pthread_mutex_lock(&queue->mutex) == 0) {
            message_t* oldEndMessage = queue->end;
            oldEndMessage->next = message;
            queue->end = message;
            queue->messageCount = queue->messageCount + 1;
            pthread_mutex_unlock(&queue->mutex);
        } else {
            logError("messageQueuePostMessage: unable to aquire mutex of the queue ! %m");
            return false;
        }
    }

    return true;
}
