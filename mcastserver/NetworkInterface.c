//
//  NetworkInterface.c
//  mcastserver
//
//  Created by Thierry DELHAISE on 06/03/2024.
//

#include "Headers/NetworkInterface.h"





const char* networkInterfaceGetFamillyName(struct sockaddr* socketAddress) {
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
        case AF_INET6:
            return "AF_INET6";
        case AF_ISDN:
            return "AF_ISDN";
#ifdef __APPLE__
        case AF_LINK:
            return "AF_LINK";
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
        case AF_NETROM:
            return "AF_NETROM";
        case AF_BRIDGE:
            return "AF_BRIDGE";
        case AF_ATMPVC:
            return "AF_ATMPVC";
        case AF_X25:
            return "AF_X25";
        case AF_ROSE:
            return "AF_ROSE";
        case AF_NETBEUI:
            return "AF_NETBEUI";
        case AF_SECURITY:
            return "AF_SECURITY";
        case AF_KEY:
            return "AF_KEY";
        case AF_PACKET:
            return "AF_PACKET";
        case AF_ASH:
            return "AF_ASH";
        case AF_ECONET:
            return "AF_ECONET";
        case AF_ATMSVC:
            return "AF_ATMSVC";
        case AF_RDS:
            return "AF_RDS";
        case AF_IRDA:
            return "AF_IRDA";
        case AF_PPPOX:
            return "AF_PPPOX";
        case AF_WANPIPE:
            return "AF_WANPIPE";
        case AF_LLC:
            return "AF_LLC";
        case AF_IB:
            return "AF_IB";
        case AF_MPLS:
            return "AF_MPLS";
        case AF_CAN:
            return "AF_CAN";
        case AF_TIPC:
            return "AF_TIPC";
        case AF_BLUETOOTH:
            return "AF_BLUETOOTH";
        case AF_IUCV:
            return "AF_IUCV";
        case AF_RXRPC:
            return "AF_RXRPC";
        case AF_PHONET:
            return "AF_PHONET";
        case AF_IEEE802154:
            return "AF_IEEE802154";
        case AF_CAIF:
            return "AF_CAIF";
        case AF_ALG:
            return "AF_ALG";
        case AF_NFC:
            return "AF_NFC";
        case AF_KCM:
            return "AF_KCM";
        case AF_QIPCRTR:
            return "AF_QIPCRTR";
        case AF_SMC:
            return "AF_SMC";
        case AF_XDP:
            return "AF_XDP";
        case AF_MCTP:
            return "AF_MCTP";
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

void networkInterfaceGetFlags(unsigned int flags, char displayFlags[MIN_DISPLAY_FLAGS_LENGTH]) {
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
    if (flags & IFF_MULTICAST) {
        strcat(displayFlags, "MULTICAST ");
    }
#ifdef __APPLE__
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
#endif
#ifdef __linux__
    if (flags & IFF_ALLMULTI) {
        strcat(displayFlags, "ALLMULTI ");
    }
    if (flags & IFF_MASTER) {
        strcat(displayFlags, "MASTER ");
    }
    if (flags & IFF_SLAVE) {
        strcat(displayFlags, "SLAVE ");
    }
    if (flags & IFF_PORTSEL) {
        strcat(displayFlags, "PORTSEL ");
    }
    if (flags & IFF_AUTOMEDIA) {
        strcat(displayFlags, "AUTOMEDIA ");
    }
    if (flags & IFF_DYNAMIC) {
        strcat(displayFlags, "DYNAMIC ");
    }
#endif
}


_Bool networkInterfaceDiscover(void) {
 
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
        
        const char* famillyName = networkInterfaceGetFamillyName(interfaceAddress->ifa_addr);
        char displayFlags[MIN_DISPLAY_FLAGS_LENGTH] = { 0 };
        networkInterfaceGetFlags(interfaceAddress->ifa_flags, displayFlags);
        fprintf(stdout,"[%s]\n\t\tfamilly [%s]\n\t\taddress [%s]\n\t\tflags   [%s]\n", interfaceAddress->ifa_name, famillyName, networkAddress, displayFlags);
    }

    freeifaddrs(interfaceAddresses);
    
    return true;
}
