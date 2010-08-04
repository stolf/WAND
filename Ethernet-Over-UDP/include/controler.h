#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "list.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <map>
#include <list>
#include <time.h>





void init_controler();
void learn_mac(ether_t mac, sockaddr_in addr, timespec* tp);
void learn_endpoint(sockaddr_in addr, timespec* tp);
#endif
