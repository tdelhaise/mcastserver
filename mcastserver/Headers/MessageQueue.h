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

typedef enum __MessageStructure {
    unknown = 0,
    multicastListenerStorage,
} message_structure_t;


typedef struct __Message {
    message_kind_t kind;
    message_structure_t structure;
    void* data;
    size_t dataLength;
    struct __Message* next;
} message_t;

typedef struct __MessageQueue {
    _Bool opened;
    pthread_mutex_t mutex;
    uint32_t messageCount;
    message_t* head;
    message_t* end;
} message_queue_t;

message_queue_t* messageQueueCreate(void);
_Bool messageQueuePostMessage(message_queue_t* queue, message_t* message);
message_t* messageQueuePeekMessage(message_queue_t* queue);
_Bool messageQueueDelete(message_queue_t* queue);
message_t* messageQueueCreateMessage(void* data, uint32_t dataLength, message_kind_t kind, message_structure_t structure);
_Bool messageQueueDeleteMessage(message_t* messageToDelete);

#endif /* MessageQueue_h */
