/* Wand Project - Ethernet Over UDP
 * $Id: mainloop.cc,v 1.26 2004/10/23 02:07:35 isomer Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#include <sys/types.h>
#include <sys/time.h> /* for timeval, select timeouts */
#include <unistd.h>
#include <string.h>
#include <signal.h> /* for sigaction (call and struct) */
#include <stdio.h> /* for fprintf and perror */
#include <map>
#include <errno.h>
#include <assert.h>
#include "debug.h"
#include "mainloop.h"

typedef std::map<int,callback_t> fd2callback_t;

static fd2callback_t fd2callback;

static fd_set rfd;
static int highestfd = 0;

volatile int endloop = 0;

/* Set SIGPIPE to SIG_IGN - if wand goes AWOL we don't really want to die 
 * return >=0 on sucess
 */
int add_sig_hnd( void )
{
	struct sigaction handler;
  
	/* Assign our_sig_hnd as our signal handler */
	handler.sa_handler = SIG_IGN;
  
	/* We don't want to block any other signals while handling this one */
	sigemptyset(&handler.sa_mask);

	/* Continously ignore this */
	handler.sa_flags = SA_RESTART;

	/* Make these values effective. */
	if (sigaction(SIGPIPE, &handler, NULL) < 0) {
		logger(MOD_INIT, 3, "Failed to add signal handler:"
			" SIGPIPE, %s\n", strerror(errno));
		return -1;
	}
	
	/* Add a handler to SIGTERM and SIGINT */
	handler.sa_handler = &sig_hnd;
	handler.sa_flags = SA_RESTART;

	sigemptyset(&handler.sa_mask);
	if (sigaction(SIGTERM, &handler, NULL) < 0) {
	  	logger(MOD_INIT, 3 , "Failed to add signal handler:"
			" SIGTERM, %s\n", strerror(errno));
	  	return -1;
	}

	sigemptyset(&handler.sa_mask);
	if (sigaction(SIGINT, &handler, NULL) < 0) {
	  	logger(MOD_INIT, 3 , "Failed to add signal handler:"
			" SIGINT, %s\n", strerror(errno));
	  	return -1;
	}
	
	return 0;
}

void sig_hnd( int signo ) 
{
  logger(MOD_IPC, 6, "Caught Signal - Ending\n");
  // Set the end flag
  endloop = 1;

}

void addRead(int fd,callback_t callback)
{
	assert(fd!=-1);
	FD_SET(fd,&rfd);
	fd2callback[fd]=callback;
	if (highestfd<fd)
		highestfd=fd;
	logger(MOD_IPC, 15, "Added fd %d, highest fd = %d\n", fd, highestfd);
}

void remRead(int fd)
{
	FD_CLR(fd,&rfd);
}

void wait_for_event(fd_set &rfd)
{
	int ret;
	fd_set rfd2;

	do {
		rfd2 = rfd;
		ret=select(highestfd+2, &rfd2, NULL, NULL, NULL);
		if (ret<0 && !endloop && errno != EINTR)
			logger(MOD_IPC, 4, "Select returned an"
				" error: %s\n", strerror(errno));
	} while (ret<0 && !endloop);

	rfd = rfd2;

}

void do_events(fd_set rfd)
{
	for (fd2callback_t::const_iterator i=fd2callback.begin(); 
			i!=fd2callback.end(); i++) {

		if (FD_ISSET(i->first,&rfd))
			i->second(i->first); /* Worst line of code -- jpc2 */
	}
}

void mainloop(void)
{
	fd_set rfd2;
	
	if( 0 > add_sig_hnd() ) {
		logger(MOD_INIT, 4, "Unable to add Signal Handler - "
			"Not Catching Signals!\n" );
	}
	while(!endloop) {
		rfd2=rfd;
		wait_for_event(rfd2);
		if (!endloop) {
			do_events(rfd2);
		}
		else {
			logger(MOD_IPC, 1, "endloop set, exiting loop\n");
		}
	}
	logger(MOD_INIT, 1, "Shutting down - breaking out of mainloop\n");
	// Close file descriptors
	for (fd2callback_t::const_iterator i=fd2callback.begin(); 
		     i!=fd2callback.end(); 
		     i++) {
		close(i->first);
	}
}
