/* Wand Project - Ethernet Over UDP
 * $Id: daemons.c,v 1.6 2002/07/07 05:37:24 isomer Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#include <sys/stat.h> /* for umask */
#include <unistd.h> /* for getpid, write, close, fork, setsid, chdir */
#include <fcntl.h> /* for creat,open */
#include <stdio.h> /* for snprintf */
#include <syslog.h> /* for openlog */
#include <string.h> /* for strrchr */
#include <stdlib.h> 

#include "daemons.h"

/* Flag set if we are a daemon or not. If we are then set to one. Used
 * to tell if we should send output to screen or syslog.
 */

int daemonised = 0;

void put_pid( char *fname )
{
	char *defname = "WandProject";
	char buf[512];
	int fd;

	if( fname == NULL ) fname = defname;

	snprintf( buf, 512, "/var/run/%s.pid", fname );
	fd=creat(buf,0660);
	if (fd<0)
		return;
	sprintf(buf,"%i\n",getpid());
	if (write(fd,buf,strlen(buf)) != (signed)strlen(buf)) {
		close(fd);
		return;
	}
	close(fd);
}

void daemonise(char *name) 
{
	
	switch (fork()) {
	case 0:
		break;
	case -1:
		perror("fork");
		exit(1);
	default:
		_exit(0);
	}
	setsid();
	switch (fork()) {
       	case 0:
       		break;
       	case -1:
       		perror("fork2");
		exit(1);
	default:
		_exit(0);
	}
	chdir("/");
	umask(0155);
	close(0);
	close(1);
	close(2);
	open("/dev/null",O_RDONLY);
	open("/dev/console",O_WRONLY);
	open("/dev/console",O_WRONLY);
	daemonised = 1;

	name = strrchr(name,'/') ? strrchr(name,'/') + 1 : name;
	
	openlog(name, LOG_PID, LOG_DAEMON);
}	
