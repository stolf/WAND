/* Wand Project - Ethernet Over UDP
 * $Id: mainloop.cc,v 1.3 2001/08/14 00:28:48 gsharp Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#include <sys/types.h>
#include <unistd.h>
#include <signal.h> /* for sigaction (call and struct) */
#include <stdio.h> /* for fprintf and perror */
#include <map>
#include "mainloop.h"

typedef map<int,callback_t> fd2callback_t;

static fd2callback_t fd2callback;

static fd_set rfd;
static int highestfd = 0;

/* Used in ui.cc to indicate we would like signals to be caught, not fatal.
 */
volatile int isWriting = 0;

static void our_sig_hnd( int signo )
{
	if(isWriting == 0 ) {
		fprintf( stderr, "Caught signal %i outside write()\n", signo );
		exit( 3 );
	}
	return;
}

/* attempt to hook our_sig_hnd in to some signals
 * return >=0 on sucess
 */
int add_sig_hnd( void )
{
	struct sigaction handler;
  
	/* Assign our_sig_hnd as our signal handler */
	handler.sa_handler = our_sig_hnd;
  
	/* We don't want to block any other signals while handling this one */
	sigemptyset(&handler.sa_mask);

	/* Make these values effective. */
	if (sigaction(SIGPIPE, &handler, NULL) < 0) {
		perror("add_sig_hnd:sigaction");
		return -1;
	}

	return 0;
}

void addRead(int fd,callback_t callback)
{
	FD_SET(fd,&rfd);
	fd2callback[fd]=callback;
	if (highestfd<fd)
		highestfd=fd;
}

void remRead(int fd)
{
	FD_CLR(fd,&rfd);
}

void mainloop(void)
{
	fd_set rfd2;
  	
	if( 0 > add_sig_hnd() ) {
		fprintf( stderr, "Unable to add Signal Handler - "
			 "Not Catching Signals!\n" );
	}
	while(1) {
		rfd2=rfd;
		select(highestfd+1,&rfd2,NULL,NULL,NULL);
		for (fd2callback_t::const_iterator i=fd2callback.begin();
		     i!=fd2callback.end();
		     i++)
			if (FD_ISSET(i->first,&rfd2))
				i->second(i->first);
	}
}