/* Wand Project - Ethernet Over UDP
 * $Id: ui.cc,v 1.6 2001/08/14 00:28:48 gsharp Exp $
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

#include "list.h"
#include "ui.h"
#include "driver.h"
#include "mainloop.h"

/* This flag is vewwy important. it is used to indicate we are trying to
 * write to a socket and would appreciate any signals being caught, not
 * killing us off.
 * No idea what signals to catch. try mainloop.cc
 */
extern volatile int isWriting;

void ui_process_callback(int fd)
{
	char buffer[1024];
	int len;
	len=read(fd,buffer,sizeof(buffer));
	if( len < 0 ) {
		perror("callback:read");
		return;
	} else if( len==0 ) {
		remRead(fd);
		close(fd);
		return;
	}
	*(buffer+len)='\0';
	char *arg=buffer;
	while( *arg && !isspace( *arg ) )
		arg++;
	while( *arg && isspace( *arg ) ) {
		*arg='\0';
		arg++;
	}

#if 0 /* Too Much Debug */
	if( arg >= (buffer+len)) 
		printf("GOT HERE! arg: past EOS buf: \"%s\"\n", buffer);
	else
		printf("GOT HERE! arg: \"%s\" buf: \"%s\"\n", arg, buffer);
#endif

	/* "ADD 00:01:02:03:04:05 1.2.3.4" */
	if (strcasecmp("add",buffer) == 0) {
		if (!arg) {
			ui_send(fd,"-ERR Missing parameter 'mac'");
			return;
		}
		char *arg2;
		arg2=arg;
		while (*arg2 && *arg2 != ' ')
			arg2++;
		if (*arg2 == '\0') {
			ui_send(fd,"-ERR Missing parameter 'ip'");
			return;
		}
		*arg2='\0';
		arg2++;
		ether_t ether;
		if( 0 > ether.parse(arg) ) {
			ui_send(fd,"-ERR MAC address does not grok");
		}
		ip_t ip;
		if ((ip=inet_addr(arg2))==-1) {
			ui_send(fd,"-ERR IP address does not grok");
			return;
		}
		add_ip(ether,ip);
		ui_send(fd,"-OK added");
		return;
	}

	/* "DEL 00:01:02:03:04:05" */
	else if (strcasecmp("del",buffer) == 0) {
		ether_t ether;
		if( 0 > ether.parse(arg) ) {
			ui_send(fd,"-ERR MAC address does not grok");
		}
		rem_ip(ether);
		ui_send(fd,"-OK deleted");
		return;
	}

	/* "LIST" */
	else if (strcasecmp("list",buffer) == 0) {
		char buffer[80];
		ui_send(fd,"+LIST ethernet\tip");
		for (online_t::const_iterator i=online.begin();
		     i!=online.end();
		     i++) 
		{
			struct sockaddr_in sockaddr;
			sockaddr.sin_addr.s_addr=i->second;
			sprintf(buffer,"+LIST %s\t%s",
				i->first(),
				inet_ntoa(sockaddr.sin_addr));
			ui_send(fd,buffer);
		}
		ui_send(fd,"-OK");
		return;
	}
	
	/* "VERSION" (Not Yet Implemented) */
	else if (strcasecmp("version",buffer) == 0) {
		ui_send(fd,"-ERR Not Supported");
		return;
	}

	/* Non blank line - complain */
	else if (strcasecmp("",buffer)!=0) {
		ui_send(fd,"-ERR Invalid or unsupported command");
		return;
	}

	return;
}

/* We would really like to catch any signals thrown in here
 */
int internal_send( int sock, char *msg, int msglen )
{
	int retval = 0;
	isWriting = 1;
	if( 0 > (retval = write(sock,msg,msglen) ) ) {
		perror( "send:write" );
		isWriting = 0;
		return retval;
	}

	isWriting = 0;
	return retval;
}

int ui_send(int sock,char *msg)
{
	if( internal_send( sock, msg, strlen(msg) ) != (int)strlen(msg) )
		return -1;
	if( internal_send( sock, "\r\n", 2 ) != 2 )
#if 1 /* Ignore errors from writing "\r\n" */
		return 0;
#else
		return -1;
#endif
	return 0;
}

static void ui_callback(int fd)
{
	int fd2=accept(fd,NULL,0);
	if (fd2>=0)
		addRead(fd2,ui_process_callback);
}

int ui_setup(char *s="/var/run/Etud.ctrl")
{
	int fd=socket(PF_UNIX,SOCK_STREAM,0);
	if (fd<0) {
		perror("control socket");
		return -1;
	}
	struct sockaddr_un sockname;
	sockname.sun_family = AF_UNIX;
	strcpy(sockname.sun_path,s);
	if (bind(fd,(const sockaddr *)&sockname,sizeof(sockname))<0) {
		perror("control bind");
		close(fd);
		return -1;
	}
	if (listen(fd,8)<0) {
		perror("control listen");
		close(fd);
		return -1;
	}
	addRead(fd,ui_callback);
	printf("UI ready\n");
	return fd;
}
