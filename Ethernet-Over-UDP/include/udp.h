/* Wand Project - Ethernet Over UDP
 * $Id: udp.h,v 1.3 2001/08/12 06:00:27 gsharp Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#ifndef UDP_H
#define UDP_H

#define UDPPORT 22222

extern int udpfd;

int udp_start(int port=UDPPORT);
int udp_read(int fd,char *buffer,int buffer_size);

#endif