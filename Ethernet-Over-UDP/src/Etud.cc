/* Wand Project - Ethernet Over UDP
 * $Id: Etud.cc,v 1.13 2002/04/18 10:33:37 isomer Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

/* <wuz> Do we need sys/time.h, sys/types.h, sys/stat.h ? */
#include <sys/time.h> /* for select */
#include <sys/types.h> /* for select, umask and open */
#include <sys/stat.h> /* for umask and open */
#include <net/ethernet.h> /* for ether_header and ETH_ALEN */
#include <iostream.h> /* for cout */
#include <dlfcn.h> /* for dlopen */
#include <unistd.h> /* for select */
#include <fcntl.h> /* for open */

#include "list.h"
#include "driver.h"
#include "udp.h"
#include "ui.h"
#include "mainloop.h"
#include "daemons.h"
#include "debug.h"
#include "config.h"

int load_module(char *s)
{
	char filename[1024];
	strcpy(filename,"/usr/local/lib/wand/");
	strcat(filename,s);
	if(!dlopen(filename,RTLD_NOW)) {
		logger(MOD_INIT, 1, "Error loading module '%s': %s\n",
				s, dlerror());
		
		return 0;
	}
	return 1;
}

int main(int arvc,char **argv)
{
  	char *driver;
	char *module;
	config_t main_config[] = {
		{ "module", TYPE_STR|TYPE_NOTNULL, &module },
		{ "driver", TYPE_STR|TYPE_NOTNULL, &driver },
	};
	if (parse_config(main_config,"/usr/local/etc/wand.conf")) {
	  logger(MOD_INIT,1,"Bad Config file, giving up\n");
	  return 1;
	}
	if (!load_module(module)) {
		logger(MOD_INIT, 1, "Aborting...\n");
		return 1;
	}
	struct interface_t *interface = find_interface(driver);
	if ((interface=find_interface(driver))==NULL) {
		logger(MOD_INIT, 1, "Failed to find driver\n");
		logger(MOD_INIT, 1, "Aborting...\n");
		return 1;
	}
	if (!init_interface(interface,1)) {
		logger(MOD_INIT, 1, "Failed to initialise interface.\n");
		logger(MOD_INIT, 1, "Aborting...\n");
		return 1;
	}
	if (udp_start()<0) {
		logger(MOD_INIT, 1, "Failed to create udp socket.\n");
		logger(MOD_INIT, 1, "Aborting...\n");
		interface->down();
		return 1;
	}
	if (ui_setup()<0) {
		logger(MOD_INIT, 1, "Failed to create unix domain socket.\n");
		interface->down();
		return 1;
	}
	logger(MOD_INIT, 6, "Etud started\n");
	logger(MOD_INIT, 6, "Using interface driver: %s\n", interface->name);
	logger(MOD_INIT, 6, " version: %s\n", interface->version);

	/* Lets go to the background */
	logger(MOD_INIT, 7, "Attempting to Daemonise...\n");
	daemonise(argv[0]);
	put_pid("Etud");

	logger(MOD_INIT, 7, "Daemonised\n");
	
	mainloop();
}
