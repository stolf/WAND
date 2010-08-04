/* Wand Project - Ethernet Over UDP
 * $Id: list.h,v 1.11 2004/01/26 08:09:14 isomer Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#ifndef LIST_H
#define LIST_H
#include <utility> /* pair<,> */
#include <list> /* list<> */
#include <string.h>
#include <sys/types.h> /* for FreeBSD */
#include <sys/socket.h> /* sockaddr_in */
#include <netinet/in.h> /*sockaddr_in */
#include <ctype.h> /* hack hack */
#include <stdio.h> /* hack hack */

class ether_t {
	public:
		uint64_t address;

		ether_t(void) {
			address = 0;
		}
		ether_t(unsigned char MAC[6]) {
			address = 0;
			memcpy(((char *)&address)+2, MAC, sizeof(address));
		}
		/* 0 on sucess; negative on failure */
		int parse(char *s) {
			/* FIXME: TODO: Throw ParseError */
			static char *digits = "0123456789abcdef";
			char *tmp;
			unsigned char* addressp = (unsigned char*)&address;
			address = 0;
			for (int i=0;i<6;i++) {
				/* Decode the first hex digit */
				if (*s == '\0') /* Need a char */
					return -1; 
				tmp=strchr(digits,tolower(*s));
				if (!tmp) /* Not a hex digit */
					return -1; 
				addressp[i+2]=tmp-digits;
			
				/* Decode the next digit */
				s++;
				if (*s == '\0') /* Need a char */
					return -1; 
				tmp=strchr(digits,tolower(*s));
				if (!tmp) /* Not a hex digit */
					return -1; 
				addressp[i+2]=(addressp[i+2]<<4)+(tmp-digits);
				s++;
	
				/* Skip intermediate : or - */
				if (*s == ':' || *s == '-')
					s++;
			}
			return 0; /* Sucess */
		}
		bool operator <(const ether_t &b) const {
			return address<b.address;
		};
		ether_t operator =(const ether_t &b) {
			address = b.address;
			return *this;
		};

		bool operator ==(const ether_t &b ) const {
		    return address==b.address;
		};

		char *operator()() const {
			unsigned char* addressp = (unsigned char*)&address;
			static char buf[18];
			sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",
				addressp[2],addressp[3],addressp[4],
				addressp[5],addressp[6],addressp[7]);
			return buf;
		};
		
		bool isBroadcast() const {
			unsigned char* addressp = (unsigned char*)&address;
			return ((addressp[2]&0x01) != 0);
		}
};


/* Maps an ethernet address to a connection (IP / port)
 * Provides no guarantee any key (ether_t) only has ONE data value (ip_t)
 */
typedef std::pair<ether_t, struct sockaddr_in > node_t;
typedef std::list<node_t> online_t;

/* Define an operator to allow sockaddr_in == sockaddr_in tests */

bool operator ==(const struct sockaddr_in a, const struct sockaddr_in b);

extern online_t online;

bool add_ip(ether_t ether, sockaddr_in addr); /* false if already existed */
bool rem_ip(ether_t ether); /* false if not found */
sockaddr_in *find_ip(ether_t ether); /* return the addr associated with ether */

/* Outputs the entire table to the given file stream.
 * Returns the number of entries so dumped
 * hack hack
 */
int dump_table( FILE *stream );

#endif

