/* Wand Project - Ethernet Over UDP
 * $Id: list.cc,v 1.16 2004/10/23 02:07:35 isomer Exp $
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
#include "debug.h"
#include <string>
#include <list>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>

bridge_table_t bridge_table;
endpoint_t endpoint_table;

bool operator !=(struct sockaddr_in a,struct sockaddr_in b) { return a.sin_port != b.sin_port || a.sin_addr.s_addr != b.sin_addr.s_addr || a.sin_family != b.sin_family; };

bool operator ==(const struct sockaddr_in a, const struct sockaddr_in b)
{
	if (a.sin_addr.s_addr != b.sin_addr.s_addr)
		return false;
	if (a.sin_port != b.sin_port)
		return false;
	if (a.sin_family != b.sin_family)
		return false;
	
	return true;
}

sockaddr_in* find_ip(ether_t mac){
  bridge_table_t::iterator it = bridge_table.find(mac);

  if (it != bridge_table.end())
	  return &(it->second.addr);
  else
	  return NULL;
}

bool add_ip(ether_t ether, struct sockaddr_in addr)
{

  int ret = 0;
  bridge_table_t::iterator it;
  endpoint_t::iterator it2;

  it = bridge_table.find(ether);
  it2 = endpoint_table.find(addr);
  bridge_entry be;
  be.addr = addr;
  be.ts.tv_sec=0;
  be.ts.tv_nsec=0;

  if( it != bridge_table.end()) {
    if(addr != it->second.addr || it->second.ts.tv_sec != 0) { /* ether and ip both match - no change */
      it->second.addr = addr;
      it->second.ts.tv_sec = 0;
      ret = 1;
    }
  }else{
    bridge_table[ether] = be;
    logger(MOD_CONTROLER, 6, "Add mac (%s, %s:%d)\n", ether(), inet_ntoa(be.addr.sin_addr), ntohs(be.addr.sin_port));
      ret = 2;
  }

  if( it2 != endpoint_table.end()) {
    if(it2->second.tv_sec != 0) {
      it2->second.tv_sec = 0;
      ret = std::max(1,ret);
    }
  }else{
    endpoint_table[addr] = be.ts;
    logger(MOD_CONTROLER, 5, "Add endpoint (%s:%d)\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
      ret = 2;
  }
  
  if (ret == 0){
    logger(MOD_LIST, 15, "\nadd_ip() Unchanged (will return f)\n");
    return false;
  }else if (ret == 1){
    logger(MOD_LIST, 15, "\nadd_ip() Changed (will return T)\n");
    return true;
  }else if (ret == 2){
    logger(MOD_LIST, 15, "\nadd_ip() Was Not In List (will return T)\n");
    return true;
  }
}

bool rem_ip(ether_t ether)
{
  bridge_table_t::iterator it;
  bridge_table_t::iterator it3;
  bridge_table_t::iterator del;
  endpoint_t::iterator it2;

  it = bridge_table.find(ether);
  if (it != bridge_table.end())
	  it2 = endpoint_table.find(it->second.addr);
  else{
	  logger(MOD_LIST, 15, "rem_ip() = FALSE\n");
	  return false;
  }


  if( it2 != endpoint_table.end() && it->second.ts.tv_sec == 0 && it2->second.tv_sec == 0) {
	  logger(MOD_LIST, 15, "rem_ip() = TRUE\n");

	  del =  bridge_table.end();
	  for(it3=bridge_table.begin(); it3!= bridge_table.end(); it3++){
		  if (del != bridge_table.end()){
			  bridge_table.erase(del);
			  del = bridge_table.end();
		  }
		  if (it3->second.addr == it->second.addr){
			  logger(MOD_CONTROLER, 6, "Delete mac (%s, %s:%d)\n", it3->first(), inet_ntoa(it3->second.addr.sin_addr), ntohs(it3->second.addr.sin_port));
			  del = it3;
		  }
	  }
	  if (del != bridge_table.end()){
		  bridge_table.erase(del);
		  del = bridge_table.end();
	  }

	  logger(MOD_CONTROLER, 5, "Delete endpoint (%s:%d)\n", inet_ntoa(it2->first.sin_addr), ntohs(it2->first.sin_port));
	  endpoint_table.erase(it2);
	  return true;
  }else{
	  logger(MOD_LIST, 15, "rem_ip() = FALSE\n");
	  return false;
  }

}

#ifdef TEST
#include <assert.h>

int main(int argc,char **argv)
{
	struct sockaddr_in test, test2;
	test.sin_family = AF_INET;
	test2.sin_family = AF_INET;
	test.sin_port = 0;
	test2.sin_port = 0;
	test.sin_addr.s_addr= 0;
	test2.sin_addr.s_addr = 0;
	
	assert(test == test2);

	test2.sin_family = AF_LOCAL;
	
	assert(test != test2);

	test2.sin_family = AF_INET;
	test2.sin_port = 1;

	assert(test != test2);

	test2.sin_port = 0;
	test2.sin_addr.s_addr = 1;

	assert(test != test2);

	test.sin_port = 22222;
	test.sin_addr.s_addr = 0x04030201;

	test2.sin_port = 22222;
	test2.sin_addr.s_addr = 0x04030202;

	
	ether_t ether;
	ether_t ether2;
	ether.parse("01:02:03:04:05:06");
	ether2.parse("01:02:03:04:05:07");
	assert(rem_ip(ether) == false);
	add_ip(ether,test);
	assert(*find_ip(ether) == test);
	assert(rem_ip(ether) != false);
	assert(rem_ip(ether) == false);
	add_ip(ether,test);
	add_ip(ether,test);
	add_ip(ether2,test2);
	assert(*find_ip(ether2) == test2);
	assert(*find_ip(ether) == test);
	assert(rem_ip(ether) != false);
	assert(rem_ip(ether2) != false);
}
#endif

