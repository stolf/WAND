/* Wand Project - Ethernet Over UDP
 * $Id: list.h,v 1.6 2001/08/14 06:27:46 gsharp Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#ifndef LIST_H
#define LIST_H
#include <map>
#include <string.h>
#include <ctype.h> /* hack hack */
#include <stdio.h> /* hack hack */

class ether_t {
	private:
		unsigned char address[6];
	public:
		ether_t(void) {
			memset(address,0,sizeof(address));
		}
		ether_t(unsigned char MAC[6]) {
			memcpy(address,MAC,sizeof(address));
		}
		/* 0 on sucess; negative on failure */
		int parse(char *s) {
			/* FIXME: TODO: Throw ParseError */
			static char *digits = "0123456789abcdef";
			char *tmp;
			for (int i=0;i<6;i++) {
				/* Decode the first hex digit */
				if (*s == '\0') /* Need a char */
					return -1; 
				tmp=strchr(digits,tolower(*s));
				if (!tmp) /* Not a hex digit */
					return -1; 
				address[i]=tmp-digits;
			
				/* Decode the next digit */
				s++;
				if (*s == '\0') /* Need a char */
					return -1; 
				tmp=strchr(digits,tolower(*s));
				if (!tmp) /* Not a hex digit */
					return -1; 
				address[i]=(address[i]<<4)+(tmp-digits);
				s++;
	
				/* Skip intermediate : or - */
				if (*s == ':' || *s == '-')
					s++;
			}
			return 0; /* Sucess */
		}
		bool operator <(const ether_t &b) const {
			for (int i=0;i<6;i++)
				if (address[i]<b.address[i])
					return true;
			return false;
		};
		ether_t operator =(const ether_t &b) {
			memcpy(address,b.address,sizeof(address));
			return *this;
		};
		
		char *operator()() const {
			static char buf[18];
			sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",
				address[0],address[1],address[2],
				address[3],address[4],address[5]);
			return buf;
		};
		
		bool isBroadcast() const {
			return ((address[0]&0x01) != 0);
		}
};

typedef int ip_t;

/* Maps an ethernet address to an ip */
typedef std::map<ether_t, ip_t > online_t;

extern online_t online;



void add_ip(ether_t ether,ip_t ip);
bool rem_ip(ether_t ether); /* false if not found */
ip_t find_ip(ether_t ether); /* return the ip associated with this ether */

#endif

