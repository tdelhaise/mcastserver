//
//  MessageQueue.h
//  mcastserver
//
//  Created by Thierry DELHAISE on 20/03/2024.
//

#ifndef MessageQueue_h
#define MessageQueue_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "Logger.h"
#include <pthread.h>

typedef enum __MessageKind {
    undefined = 0,
    incommingPacket
} message_kind_t;

typedef struct __Message {
    message_kind_t kind;
    void* data;
    size_t dataLength;
    struct __Message* next;
} message_t;

typedef struct __MessageQueue {
    pthread_mutex_t mutex;
    uint32_t messageCount;
    message_t* head;
    message_t* end;
} message_queue_t;

message_queue_t* messageQueueCreate(void);
_Bool messageQueuePostMessage(message_queue_t* queue, message_t* message);
message_t* messageQueuePeekMessage(message_queue_t* queue);

#endif /* MessageQueue_h */