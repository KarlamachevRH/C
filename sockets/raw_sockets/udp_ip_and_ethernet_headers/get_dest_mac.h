#include "headers.h"

#ifndef __GET_DEST_MAC_H__
#define __GET_DEST_MAC_H__

#define PROTO_ARP 0x0806
#define ETH2_HEADER_LEN 14
#define HW_TYPE 1
#define MAC_LENGTH 6
#define IPV4_LENGTH 4
#define ARP_REQUEST 0x01
#define ARP_REPLY 0x02
#define BUF_SIZE 60

#ifdef DEBUG
#define debug(x...) printf(x);printf("\n")
#define info(x...) printf(x);printf("\n")
#define warn(x...) printf(x);printf("\n")
#define err(x...) printf(x);printf("\n")
#else
#define debug(x...)
#define info(x...)
#define warn(x...)
#define err(x...)
#endif //DEBUG

struct arp_header 
{
    unsigned short hardware_type;
    unsigned short protocol_type;
    unsigned char hardware_len;
    unsigned char protocol_len;
    unsigned short opcode;
    unsigned char sender_mac[MAC_LENGTH];
    unsigned char sender_ip[IPV4_LENGTH];
    unsigned char target_mac[MAC_LENGTH];
    unsigned char target_ip[IPV4_LENGTH];
};

/*
 * Sample code that sends an ARP who-has request on
 * interface <ifname> to IPv4 address <ip>.
 * Returns 0 on success.
 */
void get_dest_mac(const char *ifname, in_addr_t *ip, unsigned char *sender_mac);

#endif //__GET_DEST_MAC_H__