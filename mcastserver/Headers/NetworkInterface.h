//
//  NetworkInterface.h
//  mcastserver
//
//  Created by Thierry DELHAISE on 06/03/2024.
//

#ifndef NetworkInterface_h
#define NetworkInterface_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>

_Bool networkInterfaceDiscover(void);


#endif /* NetworkInterface_h */
