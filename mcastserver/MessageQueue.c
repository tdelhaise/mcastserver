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
        
    if (queue->head == NULL && queue->messageCount != 0) {
        logError("messageQueueCheckQueueConsistancy: Queue inconsitancy ! head field can't be NULL and message count with a value different of zero !");
        return false;
    }
    
    if (queue->head == NULL && queue->end != NULL) {
        logError("messageQueueCheckQueueConsistancy: Queue inconsitancy ! head field can't be NULL and end field with a value not equal to NULL !");
        return false;
    }
    
    if( queue->head == queue->end && queue->messageCount != 1) {
        logError("messageQueueCheckQueueConsistancy: Queue inconsitancy ! head field can't be equal to end field with a messageCount != 1 !");
        return false;
    }
    
    if (queue->end != NULL && queue->end->next != NULL) {
        logError("messageQueueCheckQueueConsistancy: Queue inconsitancy ! next field of end message must be NULL !");
        return false;
    }

    return true;
}

message_t* messageQueuePeekMessage(message_queue_t* queue) {
    message_t* peekMessage = NULL;
    message_t* newHead = NULL;
    
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
