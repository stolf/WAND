/* 
 *  WANd Project - Ethernet Over UDP
 * 
 *  $Id: tuntap.c,v 1.2 2002/04/18 11:28:11 isomer Exp $
 *
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */
  
  
/*
 *  tuntap.c - Tuntap driver for Etud
 *  
 *  James Spooner < james@spoons.gen.nz >
 *
 *  read/write/setup the tuntap linux device
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <sys/socket.h>
#include <linux/if_tun.h>

#include "driver.h"

static char tapdevname[32];
static int fd = -1;

static int tuntap_setup() {

        struct ifreq ifr;
	int skfd;        

    	/* Open a socket so we can ioctl() */
        /*if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("socket");
                exit(1);
        }
        */
	printf("tuntap_setup () entered...\n");
	
	fd = 0;

	fd = open("/dev/net/tun", O_RDWR);       


	if (fd > 1) {
	
		memset(&ifr, 0, sizeof(ifr));
		
		ifr.ifr_flags = IFF_TAP;
				            
		if( (ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
		      close(fd);
		      return -1;
		}
		/*
		if (ioctl (fd, TUNSETNOCSUM, 1) < 0) {
		  close(fd);
		  fprintf(stderr, "TUNSETNOCSUM failed\n");
		}
		*/
		strcpy(tapdevname, ifr.ifr_name);
		
		fprintf(stderr, "Interface is: %s\n", ifr.ifr_name); 
		return fd;
	}
	return 0;
}

static int tuntap_read(char *frame, int length)
{
        int c;
	char buf[length+2];

	c = read(fd, buf, length + 2);
	memcpy(frame, buf+2, c - 2);
	return c - 2;
}


static int tuntap_write(char *frame, int sz) 
{
        int c;
	char buf[sz+2];
  
	bzero(buf,sz+2);
	memcpy(buf+2,frame,sz);  
	c = write (fd, buf, sz + 2);
	return c - 2;
}

static int tuntap_down(void)
{
        int skfd;
        struct ifreq ifr;
        
        
        /* Open a socket so we can ioctl() */
        
        if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("socket");
                exit(1);
        }
         
        sprintf(ifr.ifr_name, tapdevname);

        /* Set the interface to DOWN */
        ifr.ifr_flags &= ~IFF_UP; /* Set it to ~UP (down ;) */
        if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
                perror("SIOCSIFFLAGS");
                return -1;
        }       
                
        close(skfd);
        
        if (fd > 0)
                close(fd);

        return 0;
}

static struct interface_t tuntap = {
        "tuntap",
        "$Id: tuntap.c,v 1.2 2002/04/18 11:28:11 isomer Exp $",
        tuntap_setup,
        tuntap_down, 
        tuntap_read, 
        tuntap_write 
};

/* Do Dynamic initialisation */
void _init(void) {
        register_device(&tuntap);
}
        






