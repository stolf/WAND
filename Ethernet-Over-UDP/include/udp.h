/* Wand Project - Ethernet Over UDP
 * $Id: udp.h,v 1.4 2002/11/30 05:22:53 jimmyish Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#ifndef UDP_H
#define UDP_H

extern int udpfd;
extern int udpport;

int udp_start();
int udp_read(int fd,char *buffer,int buffer_size);
void udp_sendto(sockaddr_in addr,char *buffer,int size);
void udp_broadcast(char *buffer,int buffer_size, sockaddr_in* addr=NULL);

#endif
