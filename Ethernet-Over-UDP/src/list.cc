/* Wand Project - Ethernet Over UDP
 * $Id: list.cc,v 1.4 2001/08/12 06:00:27 gsharp Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

/*
 * This file maintains a list of who is connected to the network, their
 * "ethernet" address and their "ip" address.
 */

#include <map>
#include <vector>
#include <algorithm>
#include "list.h"
#include <string>
#include <list>

online_t online;

void add_ip(ether_t ether,ip_t ip)
{
	online[ether]=ip;
}

bool rem_ip(ether_t ether)
{
	if (online.find(ether)==online.end())
		return false;
	return true;
}

ip_t find_ip(ether_t ether)
{
	if (online.find(ether)==online.end())
		return false;
	return online[ether];
}

