/* Wand Project - Ethernet Over UDP
 * $Id: ui.cc,v 1.31 2003/03/06 11:58:48 mattgbrown Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#include <sys/types.h> /* for socket */
#include <sys/socket.h> /* for socket */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>


#include "list.h"
#include "controler.h"
#include "ui.h"
#include "udp.h"
#include "driver.h"
#include "mainloop.h"
#include "debug.h"
#include "etud.h"

/* "ADD mac ip port" */

static void m_add(int fd,char **argv,int argc)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	int port;
	
	if (argc<2) {
		ui_send(fd,"-ERR Not enough parameters\r\n");
		return;
	}
	ether_t ether;
	if( 0 > ether.parse(argv[1]) ) {
		ui_send(fd,"-ERR MAC address does not grok\r\n");
		return;
	}
	if (! inet_aton(argv[2], &addr.sin_addr)) {
		ui_send(fd,"-ERR IP address does not grok\r\n");
		return;
	}
	
	/* check for port > 0 & <= 65535 */
	if (strlen(argv[3]) > 5) {
		ui_send(fd,"-ERR port does not grok\r\n");
		return;
	}
	
	port = atoi(argv[3]);
	
	if (port <= 0 || port > 65535){
		ui_send(fd,"-ERR port does not grok\r\n");
		return;
	}
	
	addr.sin_port=htons((unsigned short)port);
	
	if( add_ip(ether,addr) ) {
	  ui_send(fd,"-OK updated\r\n");
	} else {
	  ui_send(fd,"-OK no change\r\n");
	}
	return;
}

/* "DEL mac" */
static void m_del(int fd,char **argv,int argc)
{
	ether_t ether;
	if (argc<1) {
		ui_send(fd,"-ERR Too few arguments\r\n");
	}
	if( 0 > ether.parse(argv[1]) ) {
		ui_send(fd,"-ERR MAC address does not grok\r\n");
	}
	rem_ip(ether);
	ui_send(fd,"-OK deleted\r\n");
	return;
}

/* GETMAC */
static void m_getmac(int fd,char **argv,int argc)
{
	char tbuff[80];
	sprintf(tbuff,"+GETMAC %s\r\n", macaddr);
	ui_send(fd, tbuff);
	ui_send(fd, "-OK\r\n");

}

/* GETPORT */
static void m_getport(int fd,char **argv,int argc)
{
	char tbuff[80];
	sprintf(tbuff,"+GETPORT %u\r\n", udpport);
	ui_send(fd, tbuff);
	ui_send(fd, "-OK\r\n");

}

/* LIST */
static void m_list(int fd,char **argv,int argc)
{
	char tbuff[80];
	ui_send(fd,"+LIST ethernet\tip\tport\r\n");
	
	logger(MOD_IPC, 15, "About to perform list\n");
	for (online_t::const_iterator i=online.begin();
	     i!=online.end();
	     i++) 
	{
		logger(MOD_IPC, 15, "+LIST %s\t%s\t%d\r\n",
			i->first(),
			inet_ntoa(i->second.sin_addr), 
			ntohs(i->second.sin_port));

		sprintf(tbuff,"+LIST %s\t%s\t%d\r\n",
			i->first(),
			inet_ntoa(i->second.sin_addr), 
			ntohs(i->second.sin_port));
		ui_send(fd,tbuff);
	}
	logger(MOD_IPC, 15, "Finished list\n");
	ui_send(fd,"-OK\r\n");
	return;
}

/* TLIST */
static void c_list(int fd,char **argv,int argc)
{
	char tbuff[80];
	timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);

	ui_send(fd,"+TLIST ethernet\tip\tport\tremain\r\n");
	
	logger(MOD_IPC, 15, "About to perform tlist\n");
	for (bridge_table_t::const_iterator i=bridge_table.begin();
	     i!=bridge_table.end();
	     i++) 
	{
		logger(MOD_IPC, 15, "+TLIST %s\t%s\t%d\t%d\r\n",
			i->first(),
			inet_ntoa(i->second.addr.sin_addr), 
			ntohs(i->second.addr.sin_port),
			i->second.ts.tv_sec - tp.tv_sec);

		sprintf(tbuff,"+TLIST %s\t%s\t%d\t%d\r\n",
			i->first(),
			inet_ntoa(i->second.addr.sin_addr), 
			ntohs(i->second.addr.sin_port),
			i->second.ts.tv_sec - tp.tv_sec);
		ui_send(fd,tbuff);
	}
	logger(MOD_IPC, 15, "Finished tlist\n");
	ui_send(fd,"-OK\r\n");
	return;
}
/* ELIST */
static void e_list(int fd,char **argv,int argc)
{
	char tbuff[80];
	timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);

	ui_send(fd,"+ELIST ip\tport\tremain\r\n");
	
	logger(MOD_IPC, 15, "About to perform Elist\n");
	for (endpoint_t::const_iterator i=endpoint_table.begin();
	     i!=endpoint_table.end();
	     i++) 
	{
		logger(MOD_IPC, 15, "+ELIST %s\t%d\t%d\r\n",
			inet_ntoa(i->first.sin_addr), 
			ntohs(i->first.sin_port),
			i->second.tv_sec - tp.tv_sec);

		sprintf(tbuff,"+ELIST %s\t%d\t%d\r\n",
			inet_ntoa(i->first.sin_addr), 
			ntohs(i->first.sin_port),
			i->second.tv_sec - tp.tv_sec);
		ui_send(fd,tbuff);
	}
	logger(MOD_IPC, 15, "Finished elist\n");
	ui_send(fd,"-OK\r\n");
	return;
}

static void m_version(int fd,char **argv,int argc)
{
	ui_send(fd,"-ERR Not Supported\r\n");
	return;
}

struct functable_entry_t {
	char *name;
	void (*func)(int fd,char **,int);
}; 

static struct functable_entry_t functable[] = {
	{ "ADD", m_add },
	{ "DEL", m_del },
	{ "GETMAC", m_getmac },
	{ "GETPORT", m_getport },
	{ "LIST", m_list },
	{ "TLIST", c_list },
	{ "ELIST", e_list },
	{ "VERSION", m_version },
	{ NULL, NULL }
};

/* Parses a line */
static int parse(char *line,char **argv,int argc)
{
	int arg=0;
	argv[0] = line;
	while (*line && arg<argc) {
		if (*line == ' ') {
			*line='\0';
			arg++;
			argv[arg] = line+1;
		}
		line++;
	}
	return arg;
}

#define MAX_ARGS 5
void ui_process_callback(int fd)
{
	char buffer[1024];
	int len;
	char *argv[MAX_ARGS];
	functable_entry_t *functable_iter;

	len=read(fd,buffer,sizeof(buffer));
	if( len < 0 ) {
		logger(MOD_IPC, 5, "Read error in callback: %s\n",
				strerror(errno));
		return;
	} else if( len==0 ) {
		remRead(fd);
		close(fd);
		return;
	}
	*(buffer+len)='\0';

  // check for extra cruft

  //step from end of buffer, removing \r\n 
  //at the moment, this will kill all \r\n
  
  while (len >= 0) {
    if (buffer[len] == '\r') {
      buffer[len] = '\0';
    } else if (buffer[len] == '\n') {
      buffer[len] = '\0';
    }
    len--;
  }
  
  if (strcmp(buffer,"") == 0)
    return;
		
  //logger(MOD_IPC, 5, "Parsing %s\n", buffer);
   
	int argc=parse(buffer,argv,MAX_ARGS);

	
	for (functable_iter = functable;functable_iter->name;functable_iter++) {
		if (strcasecmp(functable_iter->name,argv[0]) == 0) {
			functable_iter->func(fd,argv,argc);
			return;
		}
	}
	ui_send(fd,"-ERR Unknown command\r\n");
	return;
}

int internal_send( int sock, char *msg, int msglen )
{
	int retval = 0;
	if( 0 > (retval = write(sock,msg,msglen) ) ) {
		logger(MOD_IPC, 5, "Write error in send: %s\n",
				strerror(errno));
		return retval;
	}

	return retval;
}

int ui_send(int sock,char *msg)
{
	if( internal_send( sock, msg, strlen(msg) ) != (int)strlen(msg) )
		return -1;
#if 0
	if( internal_send( sock, "\r\n", 2 ) != 2 )
#if 1 /* Ignore errors from writing "\r\n" */
		return 0;
#else
		return -1;
#endif
#endif
		
	return 0;
}

static void ui_callback(int fd)
{
	int fd2=accept(fd,NULL,0);
	if (fd2>=0){
		addRead(fd2,ui_process_callback);
		logger(MOD_IPC, 15, "UI accept succeeded\n");

	}
}

int ui_setup(char *s)
{
	int fd=socket(PF_UNIX,SOCK_STREAM,0);
	if (fd<0) {
		logger(MOD_IPC, 1, "Error creating control socket: %s\n",
				strerror(errno));
		return -1;
	}
	struct sockaddr_un sockname;
	sockname.sun_family = AF_UNIX;
	strcpy(sockname.sun_path,s);
	if (bind(fd,(const sockaddr *)&sockname,sizeof(sockname))<0) {
		logger(MOD_IPC, 1, "Error binding to control socket: %s\n",
				strerror(errno));
		close(fd);
		return -1;
	}
	if (listen(fd,8)<0) {
		logger(MOD_IPC, 1, "Error listening to control socket: %s\n",
				strerror(errno));
		close(fd);
		return -1;
	}
	addRead(fd,ui_callback);
	logger(MOD_IPC, 7, "UI ready\n");
	return fd;
}
