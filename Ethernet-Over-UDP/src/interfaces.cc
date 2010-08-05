/* Wand Project - Ethernet Over UDP
 * $Id: interfaces.cc,v 1.19 2003/07/21 06:35:17 jspooner Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#include "controler.h"
#include "driver.h"
#include "list.h"
#include "udp.h"
#include "mainloop.h"
#include <net/ethernet.h>
#include <netinet/in.h>
#include <vector>
#include <map>
#include <errno.h>
#include <sys/socket.h>

#ifdef LINUX
#include <linux/sockios.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#else
#include <net/if.h>
#include <net/if_arp.h>
#endif

#include <sys/ioctl.h>
#include <unistd.h>

#include "debug.h"
#include "etud.h"

struct ether_header_t {
	unsigned short int fcs;
        struct ether_header eth;
};

struct interface_t *driver;

extern "C" {

  void register_device(struct interface_t *interface)
  {
	driver = interface;
  }

}

const int BUFFERSIZE = 65536;

void udp_sendto(sockaddr_in addr,char *buffer,int size)
{
	sendto(udpfd,buffer,size,0,(const struct sockaddr *)&addr,sizeof(addr));
}

void udp_broadcast(char* buffer, int size, sockaddr_in* addr)
{
	//logger(MOD_IF,15,"Broadcasting\n");
	for (endpoint_t::const_iterator i=endpoint_table.begin();
		 i!=endpoint_table.end();
		 i++){
		if (addr == NULL || !(i->first == *addr))
			udp_sendto(i->first,buffer,size);
	}
}

static void do_read(int fd)
{
	static char buffer[BUFFERSIZE];
	int size=driver->read(buffer,BUFFERSIZE);
	if (size<16) {
		logger(MOD_IF,4,"Runt, Oink! Oink! %i<16\n",size);
		return;
	};
	ether_t ether(((ether_header_t *)(buffer))->eth.ether_dhost);
	if (ether.isBroadcast()) {
		udp_broadcast(buffer, size);
	}
	else {
		sockaddr_in *addr = find_ip(ether);
		if (addr) {
			logger(MOD_IF,15,"Sending %s to %i\n",ether(),
					addr->sin_addr);
			udp_sendto(*addr,buffer,size);
		}
		else {
			logger(MOD_IF,10,"No match for %s\n",ether());
			if (forward_unknown)
				udp_broadcast(buffer, size);
		};
	};
}

int init_interface(void)
{

	char buf[1024];
	struct ifreq ifr;
	struct ifconf ifc;
	struct ifreq* ifa;
	int skfd, iface_count;
	ether_t ether;


	/* Open a socket so we can ioctl() */
	//if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	if ((skfd = socket(PF_PACKET, SOCK_RAW, 0)) < 0) {
	    logger(MOD_IF, 1, "Socket create failed - %m\n");
	    return 0;
        }			

	int ifd;
	if ((ifd=driver->setup(ifname))<=0) {
		return 0;
	}
	
	/* Configure the MAC Address on the interface */
	snprintf(ifr.ifr_name, IFNAMSIZ, "%s", ifname);

	/* If mac not specified, borrow from the first interface that has an
	 * IP address and a ethernet address
	*/
	if (macaddr == NULL){
		logger(MOD_IF, 15, "No maccaddr specified, trying to find one\n");
		ifc.ifc_len = sizeof(buf);
		ifc.ifc_buf = buf;
		if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0){
			logger(MOD_IF, 1, "Get interfaces failed, %s\n", strerror(errno));
			return 0;
		}
		ifa = ifc.ifc_req;
		iface_count = ifc.ifc_len / sizeof(ifr);
		for(int i = 0; i < iface_count; i++){
			ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
			if(ioctl(skfd, SIOCGIFHWADDR, &ifa[i]) < 0) {
				logger(MOD_IF, 1, "Socket GET MAC Address failed \n");
				return 0;
			}
	        memcpy(((char*)&ether.address)+2, &ifa[i].ifr_addr.sa_data, 6);
			logger(MOD_IF, 15, "Found IFace %s(%s)\n", ether(), ifa[i].ifr_name);
			if (ether.address != 0){
				logger(MOD_IF, 6, "Mac not specified, using %s(%s)\n", ether(), ifa[i].ifr_name);
				break;
			}
		}
		if (ether.address == 0){
			logger(MOD_IF, 1, "Mac not specified, and cannot find one to use\n");
			return 0;
		}
	}else{
		ether.parse(macaddr);
	}

#ifdef LINUX
	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	memcpy(&ifr.ifr_hwaddr.sa_data, ((char*)&ether.address)+2, 6);
	if(ioctl(skfd, SIOCSIFHWADDR, &ifr) < 0) {
		logger(MOD_IF, 1, "Socket Set MAC Address failed - %m\n");
		return 0;
	}
#else
        ifr.ifr_addr.sa_len = ETHER_ADDR_LEN;
        ifr.ifr_addr.sa_family = AF_LINK;
        memcpy(&ifr.ifr_addr.sa_data, ((char*)&ether.address)+2, 6);
        if (ioctl(skfd, SIOCSIFLLADDR, (caddr_t)&ifr) < 0) {
                        logger(MOD_IF,1,"ioctl (set lladdr)");
                        return 0;
        }
#endif
	
	/* Set ARP and MULTICAST on the interface */
  /* Read the current flags on the interface */
  if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
    logger(MOD_IF, 1, "Get Flags failed on device - %m\n");
    return 0;
  }
  /* remove the NOARP, set the MULTICAST flags */
  ifr.ifr_flags &= ~IFF_NOARP;
  ifr.ifr_flags |= IFF_MULTICAST;
  
  /* commit changes */
  if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
    logger(MOD_IF, 1, "Set Flags failed on device - %m\n");
    return 0;
  }
  
  /* Set MTU on the interface  */
  ifr.ifr_mtu = mtu; 
  if(ioctl(skfd, SIOCSIFMTU, &ifr) < 0) {
    logger(MOD_IF, 1, "Socket Set MTU failed - %m\n");
    return 0;
  }
  
  
	close(skfd);
	
	addRead(ifd,do_read);
	return 1;
}

int shutdown_interface(void)
{
	int ifd;
	if ((ifd=driver->down())<=0) {
		return 0;
	}
	remRead(ifd);
	return 1;
}
void send_interface(char *buffer,int size)
{
	driver->write(buffer,size);
}
