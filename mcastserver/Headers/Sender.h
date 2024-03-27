//
//  Sender.h
//  mcastserver
//
//  Created by Thierry DELHAISE on 24/03/2024.
//

#ifndef Sender_h
#define Sender_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <sys/syslog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "Logger.h"
#include "MessageQueue.h"
#include "Resolver.h"


_Bool senderRun(void);
_Bool senderPrepareRun(message_queue_t* serverMessageQueue);
void senderShutdown(void);
void senderStop(void);
void senderWaitForTermination(void);



#endif /* Sender_h */
