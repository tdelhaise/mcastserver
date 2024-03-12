//
//  NetworkInterface.c
//  mcastserver
//
//  Created by Thierry DELHAISE on 06/03/2024.
//

#include "Headers/NetworkInterface.h"





const char* NetworkInterfaceGetFamillyName(struct sockaddr* socketAddress) {
    if (socketAddress == NULL) {
        return "ERROR";
    }


    switch (socketAddress->sa_family) {
        case AF_UNSPEC:
            return "AF_UNSPEC";
        case AF_UNIX:
            return "AF_UNIX";
        case AF_INET:
            return "AF_INET";
        case AF_SNA:
            return "AF_SNA";
        case AF_DECnet:
            return "AF_DECnet";
        case AF_APPLETALK:
            return "AF_APPLETALK";
        case AF_ROUTE:
            return "AF_ROUTE";
        case AF_IPX:
            return "AF_IPX";
        case AF_LINK:
            return "AF_LINK";
        case AF_INET6:
            return "AF_INET6";
        case AF_ISDN:
            return "AF_ISDN";
#ifdef __APPLE__
        case AF_IMPLINK:
            return "AF_IMPLINK";
        case AF_PUP:
            return "AF_PUP";
        case AF_CHAOS:
            return "AF_CHAOS";
        case AF_NS:
            return "AF_NS";
        case AF_ISO:
            return "AF_ISO";
        case AF_ECMA:
            return "AF_ECMA";
        case AF_DATAKIT:
            return "AF_DATAKIT";
        case AF_CCITT:
            return "AF_CCITT";
        case AF_DLI:
            return "AF_DLI";
        case AF_LAT:
            return "AF_LAT";
        case AF_HYLINK:
            return "AF_HYLINK";
        case pseudo_AF_XTP:
            return "pseudo_AF_XTP";
        case AF_COIP:
            return "AF_COIP";
        case AF_CNT:
            return "AF_CNT";
        case pseudo_AF_RTIP:
            return "pseudo_AF_RTIP";
        case AF_SIP:
            return "AF_SIP";
        case pseudo_AF_PIP:
            return "pseudo_AF_PIP";
        case AF_NDRV:
            return "AF_NDRV";
        case pseudo_AF_KEY:
            return "pseudo_AF_KEY";
        case AF_NATM:
            return "AF_NATM";
        case AF_SYSTEM:
            return "AF_SYSTEM";
        case AF_NETBIOS:
            return "AF_NETBIOS";
        case AF_PPP:
            return "AF_PPP";
        case pseudo_AF_HDRCMPLT:
            return "pseudo_AF_HDRCMPLT";
        case AF_RESERVED_36:
            return "AF_RESERVED_36";
        case AF_IEEE80211:
            return "AF_IEEE80211";
        case AF_UTUN:
            return "AF_UTUN";
#endif
#ifdef __linux__
        case IFF_ALLMULTI:
            return "IFF_ALLMULTI";
        case IFF_MASTER:
            return "IFF_MASTER";
        case IFF_SLAVE:
            return "IFF_SLAVE";
        case IFF_MULTICAST:
            return "IFF_MULTICAST";
        case IFF_PORTSEL:
            return "IFF_PORTSEL";
        case IFF_AUTOMEDIA:
            return "IFF_AUTOMEDIA";
        case IFF_DYNAMIC:
            return "IFF_DYNAMIC";
#endif
        case AF_VSOCK:
            return "AF_VSOCK";
        case AF_MAX:
            return "AF_MAX";
        default:
            return "UNKNOWN";
    }
}

#define MIN_DISPLAY_FLAGS_LENGTH    15*16

void NetworkInterfaceGetFlags(unsigned int flags, char displayFlags[MIN_DISPLAY_FLAGS_LENGTH]) {
    if (flags & IFF_BROADCAST) {
        strcat(displayFlags, "BROADCAST ");
    }
    if (flags & IFF_POINTOPOINT) {
        strcat(displayFlags, "POINTOPOINT ");
    }
    if (flags & IFF_UP) {
        strcat(displayFlags, "UP ");
    }
    if (flags & IFF_DEBUG) {
        strcat(displayFlags, "DEBUG ");
    }
    if (flags & IFF_LOOPBACK) {
        strcat(displayFlags, "LOOPBACK ");
    }
    if (flags & IFF_NOTRAILERS) {
        strcat(displayFlags, "NOTRAILERS ");
    }
    if (flags & IFF_RUNNING) {
        strcat(displayFlags, "RUNNING ");
    }
    if (flags & IFF_NOARP) {
        strcat(displayFlags, "NOARP ");
    }
    if (flags & IFF_PROMISC) {
        strcat(displayFlags, "PROMISC ");
    }
    if (flags & IFF_ALLMULTI) {
        strcat(displayFlags, "ALLMULTI ");
    }
    if (flags & IFF_OACTIVE) {
        strcat(displayFlags, "OACTIVE ");
    }
    if (flags & IFF_SIMPLEX) {
        strcat(displayFlags, "SIMPLEX ");
    }
    if (flags & IFF_LINK0) {
        strcat(displayFlags, "LINK0 ");
    }
    if (flags & IFF_LINK1) {
        strcat(displayFlags, "LINK1 ");
    }
    if (flags & IFF_LINK2) {
        strcat(displayFlags, "LINK2 ");
    }
    if (flags & IFF_MULTICAST) {
        strcat(displayFlags, "MULTICAST");
    }
}


_Bool discoverNetworkInterface(void) {
 
    struct ifaddrs *interfaceAddresses, *interfaceAddress;

    if (getifaddrs(&interfaceAddresses) == -1) {
        perror("getifaddrs");
        return false;
    }

    /* Walk through linked list, maintaining head pointer so we
     can free list later */
    for (interfaceAddress = interfaceAddresses; interfaceAddress != NULL; interfaceAddress = interfaceAddress->ifa_next) {
        if (interfaceAddress->ifa_addr == NULL) {
            fprintf(stdout,"Ignoring interface: %s since its address is null\n", interfaceAddress->ifa_name);
            continue;
        }
        
        char networkAddress[INET6_ADDRSTRLEN] = { 0 };
        if (interfaceAddress->ifa_addr->sa_family == AF_INET) {
            struct in_addr ip_address = ((struct sockaddr_in*)interfaceAddress->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, &ip_address, networkAddress, INET6_ADDRSTRLEN);
        }
        if (interfaceAddress->ifa_addr->sa_family == AF_INET6) {
            struct in6_addr ip6_address = ((struct sockaddr_in6*)interfaceAddress->ifa_addr)->sin6_addr;
            inet_ntop(AF_INET6, &ip6_address, networkAddress, INET6_ADDRSTRLEN);
        }
        
        const char* famillyName = NetworkInterfaceGetFamillyName(interfaceAddress->ifa_addr);
        char displayFlags[MIN_DISPLAY_FLAGS_LENGTH] = { 0 };
        NetworkInterfaceGetFlags(interfaceAddress->ifa_flags, displayFlags);
        fprintf(stdout,"[%s]\n\t\tfamilly [%s]\n\t\taddress [%s]\n\t\tflags   [%s]\n", interfaceAddress->ifa_name, famillyName, networkAddress, displayFlags);
    }

    freeifaddrs(interfaceAddresses);
    
    return true;
}
