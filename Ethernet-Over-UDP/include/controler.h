#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "list.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <map>
#include <list>

struct bridge_entry {
	sockaddr_in addr;
	timespec ts;
};

struct ltaddr
{
	bool operator()(const sockaddr_in& s1, const sockaddr_in& s2)
	{
		if (s1.sin_addr.s_addr == s2.sin_addr.s_addr){
			return s1.sin_port < s2.sin_port;
		}else{
			return s1.sin_addr.s_addr < s2.sin_addr.s_addr;
		}
	}
};


typedef std::map<ether_t, struct bridge_entry> bridge_table_t;
typedef std::map<sockaddr_in, timespec, ltaddr> endpoint_t;
extern bridge_table_t bridge_table;
extern endpoint_t endpoint_table;

extern int do_controler;
extern int do_relay_broadcast;
extern int controler_mac_age;
extern int controler_endpoint_age;

void init_controler();
void learn_mac(ether_t mac, sockaddr_in addr);
void controler_broadcast(char* buffer, int size);
sockaddr_in* controler_find_ip(ether_t mac);
#endif
