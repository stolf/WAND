/* Wand Project - Ethernet Over UDP
 * $Id: 
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#ifndef ETUD_H
#define ETUD_H

extern char *macaddr;
extern char *ifname;
extern int mtu;
extern int forward_unknown;
extern int dynamic_mac;
extern int dynamic_endpoint;
extern int no_endpoint_discard;
extern int relay_broadcast;
extern int controler_mac_age;
extern int controler_endpoint_age;
#endif
