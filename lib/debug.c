/* Wand Project - Ethernet Over UDP
 * $Id: debug.c,v 1.13 2003/01/31 11:12:10 jimmyish Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#include <stdio.h> /* printf */
#include <syslog.h> /* sylog */
#include <stdlib.h> /* free */
#include <stdarg.h> /* va */
#include <assert.h> /* assert */

#include "debug.h"
#include "daemons.h"

static int loglookup[]={LOG_ALERT, LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING,
		 LOG_NOTICE, LOG_INFO, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG,
		 LOG_DEBUG, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG,
		 LOG_DEBUG };

/* This defines the level at which each module is requesting be printed.
 * If the level passed to logger is higher than the level specified in this
 * array then we do not print the message.
 *
 * Initialise to -1 which means each level uses the default specified in
 * default_debug_level.
 */

int modtolevel[]= {-1, -1, -1, -1, -1, -1, -1, -1, -1};

/* Make the default log level 7, all non debug info. */
int default_log_level = 7;

extern int daemonised;

/* See debug.h for a description of the module and level definations. */

void logger(int module, int level, const char *format, ...)
{
	va_list ap;
	char buffer[513];
	int module_level;
	
	/* Sanity checks. If these fail it's the person who is calling us
	 * at fault !
	 */
	
	assert(default_log_level <= 15 && default_log_level >= 0);
	assert(level <= 15 && level >= 0);
	assert(module <= 8 && module >= 0);
	assert(daemonised == 0 || daemonised == 1);
	assert(modtolevel[module] <= 15 && modtolevel[module] >= -1);
	
	va_start(ap, format);

	/* Check if the module is being logged at level -1, if it is,
	 * this module is actually being logged at the default level. */

	if(modtolevel[module] < 0){
		module_level = default_log_level;
	} else {
		module_level = modtolevel[module];
	}
	
	if(level <= module_level){
		if(! daemonised){
			if(level < 7){
				vfprintf(stderr, format, ap);
			} else {
				vprintf(format, ap);
			}
		} else {
			vsnprintf(buffer, sizeof(buffer), format, ap);
			syslog(loglookup[level], "%s", buffer);
		}
	}

	va_end(ap);
}
	
