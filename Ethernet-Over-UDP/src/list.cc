/* Wand Project - Ethernet Over UDP
 * $Id: list.cc,v 1.14 2003/03/08 22:06:15 isomer Exp $
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

online_t online;

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


bool add_ip(ether_t ether, struct sockaddr_in addr)
{
  if( !online.empty() ) {
    online_t::iterator i = online.begin();
    logger(MOD_LIST, 15, "\nadd_ip(%s, %x) Entered\n", ether(), addr.sin_addr);
    while( i != online.end() ) { /* Not empty, search for given ether */
      if( ((*i).first) == ether ) { /* match, handle */
	if(addr == (*i).second) { /* ether and ip both match - no change */
	  logger(MOD_LIST, 15, "\nadd_ip() Unchanged (will return f)\n");
	  return false;
	} else { /* node has changed, update it */
	  (*i).second = addr;
	  logger(MOD_LIST, 15, "\nadd_ip() Changed (will return T)\n");
	  return true;
	}
      }
      ++i;
    }
    /* given ether is not in list, add it */
    logger(MOD_LIST, 15, "\nadd_ip() Was Not In List (will return T)\n");
    online.push_back( node_t::pair(ether,addr) );
    return true;
  } else { /* List was empty, add the node */
    logger(MOD_LIST, 15, "\nadd_ip(%s, %x) Was Empty (will return T)\n", 
		    ether(), addr.sin_addr);
    online.push_back( node_t::pair(ether,addr) );
    return true;
  }
  /* should never get here */
  assert(0);
}

bool rem_ip(ether_t ether)
{
  if (online.empty() ) {
    logger(MOD_LIST, 15, "rem_ip() Empty\n");
    /* No need to dump the table -- it's empty :) */
    return false;
  }
  logger(MOD_LIST, 15, "\nrem_ip(%s) Entered\n", ether() );
  online_t::iterator i = online.begin();
  bool found_and_removed = false;
  while( i != online.end() ) {
    if( ((*i).first) == ether ) { /* match, remove */
      found_and_removed = true;
      online.remove( *i );
      break;
    }
    ++i;
  }
  logger(MOD_LIST, 15, "rem_ip() = %s\n", (found_and_removed)?"TRUE":"FALSE");
  return found_and_removed;
}

sockaddr_in *find_ip(ether_t ether)
{
  sockaddr_in *found = NULL;
  
  if (online.empty() ) {
    logger(MOD_LIST, 15, "find_ip() Empty\n");
    /* No need to dump the table -- it's empty :) */
    return false;
  }
  logger(MOD_LIST, 15, "\nfind_ip(%s) Entered\n", ether());

  online_t::iterator i = online.begin();
  
  while( i != online.end() ) {
    if( ((*i).first) == ether ) { /* match, return */
      found = &(*i).second;
      break;
    }
    ++i;
  }
  logger(MOD_LIST, 15, "find_ip() = %x\n", found->sin_addr);
  return found;
}

int dump_table( FILE *stream )
{
  if( online.empty() ) {
    fprintf( stream, "Table is empty.\n" );
    return 0;
  }
  online_t::iterator i = online.begin();
  int c = 0;
  while( i != online.end() ) {
    fprintf( stream, "%2i: online[\"%s\"] = %x, %d\n", c,
	     ((*i).first)(), (*i).second.sin_addr.s_addr, 
	     (*i).second.sin_port );
    ++i;
    ++c;
  }
  fprintf( stream, "--\n" );
  return c;
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

