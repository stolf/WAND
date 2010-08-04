#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "list.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <map>
#include <list>




extern int do_controler;
extern int do_relay_broadcast;
extern int controler_mac_age;
extern int controler_endpoint_age;

void init_controler();
void learn_mac(ether_t mac, sockaddr_in addr);
#endif
