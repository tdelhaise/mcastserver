//
//  Logger.h
//  mcastserver
//
//  Created by Thierry DELHAISE on 20/03/2024.
//

#ifndef Logger_h
#define Logger_h

#include <syslog.h>


#define logInfo(...) syslog(LOG_INFO, __VA_ARGS__)
#define logDebug(...) syslog(LOG_DEBUG, __VA_ARGS__)
#define logWarning(...) syslog(LOG_WARNING, __VA_ARGS__)
#define logError(...) syslog(LOG_ERR, __VA_ARGS__)


#endif /* Logger_h */
