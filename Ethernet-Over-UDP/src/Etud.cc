/* Wand Project - Ethernet Over UDP
 * $Id: Etud.cc,v 1.55 2004/10/23 02:07:34 isomer Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

/* <wuz> Do we need sys/time.h, sys/types.h, sys/stat.h ? */
#include <sys/time.h> /* for select */
#include <sys/types.h> /* for select, umask and open */
#include <sys/stat.h> /* for umask and open */
#include <net/ethernet.h> /* for ether_header and ETH_ALEN */
#include <dlfcn.h> /* for dlopen */
#include <unistd.h> /* for select */
#include <fcntl.h> /* for open */
#include <getopt.h> /* for parsing command line options */
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h> /* for basename */


#include "controler.h"
#include "list.h"
#include "driver.h"
#include "udp.h"
#include "ui.h"
#include "mainloop.h"
#include "daemons.h"
#include "debug.h"
#include "config.h"

extern int modtolevel[];
extern int default_log_level;
char *macaddr=NULL;
char *ifname=NULL;
int mtu=1280;
int dynamic_mac = 1;
int no_endpoint_discard = 1;
int dynamic_endpoint = 0;
int forward_unknown = 0;
int relay_broadcast = 0;
int controler_mac_age = 300;
int controler_endpoint_age = 900;

int load_module(char *filename)
{
	if(!dlopen(filename,RTLD_NOW)) {
		logger(MOD_INIT, 1, "Error loading module '%s': %s\n",
				filename , dlerror());
		
		return 0;
	}
	return 1;
}

void usage(const char *prog) {
	char *progname;

	progname=strdup(prog);
	
	printf("%s:	[-d module]	- Transport driver to use\n"
"	[-a]		- Set mac age time in seconds\n"
"	[-n]		- No dynamic mac learning\n"
"	[-N]		- No discard when there is no endpoint\n"
"	[-D]		- Don't daemonise\n"
"	[-e]		- Do dynamic endpoint learning\n"
"	[-E]		- Set endpoint age time in seconds\n"
"	[-f configfile]	- Read config from this file\n"
"	[-F]		- Forward (broadcast) unknown mac addresses\n"
"	[-h]		- This help\n"
"	[-i ifname]	- Name of the interface to create \n"
"	[-l port]	- Communicate on the specified port\n"
"	[-L level]	- Default logging level, 0 = silent, 15 = noisy\n"
"	[-m macaddr]	- MAC address for the created interface\n"
"  	[-M mtu] - MTU for the created interface\n"
"	[-p pidfile]	- File to store pid in\n"
"	[-r]		- Relay broadcast if tunnel controler\n"
"\n"
"Options on command line override those in the config file.\n", 
	basename(progname));

}

int main(int argc,char **argv)
{
	char buf[1024];
	
	/* Actual configuration options */
	int do_daemonise=1;
	char *module=NULL;
	char *conffile=NULL;
	char *pidfile = strdup("/var/run/Etud.pid");
	char *ctrlfile=NULL;
  
	/* Temporary options read from the command line */
	char *cmacaddr=NULL;
	char *cmodule=NULL;
	char *cpidfile=NULL;
	char *cctrlfile=NULL;
	char *cifname=NULL;
	int cdo_daemonise=1;
	int cdynamic_mac=1;
	int cdynamic_endpoint=0;
	int cno_endpoint_discard = 1;
	int cforward_unknown=0;
	int crelay_broadcast=0;
	int ccontroler_mac_age = -1;
	int ccontroler_endpoint_age = -1;
	int cudpport=-1;
  	int cmtu=-1;
	int clevel=-1;

	/* Possible config file options */
	config_t main_config[] = {
		{ "module", TYPE_STR|TYPE_NOTNULL, &module },
		{ "daemonise", TYPE_BOOL|TYPE_NULL, &do_daemonise },
		{ "dynamic_mac", TYPE_BOOL|TYPE_NULL, &dynamic_mac },
		{ "dynamic_endpoint", TYPE_BOOL|TYPE_NULL, &dynamic_endpoint },
		{ "relay_broadcast", TYPE_BOOL|TYPE_NULL, &relay_broadcast },
		{ "no_endpoint_discard", TYPE_BOOL|TYPE_NULL, &no_endpoint_discard },
		{ "forward_unknown", TYPE_BOOL|TYPE_NULL, &forward_unknown },
		{ "mac_age", TYPE_INT|TYPE_NULL, &controler_mac_age },
		{ "endpoint_age", TYPE_INT|TYPE_NULL, &controler_endpoint_age },
		{ "macaddr", TYPE_STR|TYPE_NULL, &macaddr },
		{ "ifname", TYPE_STR|TYPE_NULL, &ifname },
		{ "pidfile", TYPE_STR|TYPE_NULL, &pidfile },
		{ "udpport", TYPE_INT|TYPE_NULL, &udpport },
		{ "ctrlfile", TYPE_STR|TYPE_NULL, &ctrlfile },
 		{ "mtu", TYPE_INT|TYPE_NULL, &mtu },
		{ "debug_default", TYPE_INT|TYPE_NULL, &default_log_level},
		{ "debug_MOD_DRIVERS", TYPE_INT|TYPE_NULL, &modtolevel[MOD_DRIVERS]},
		{ "debug_MOD_IF", TYPE_INT|TYPE_NULL, &modtolevel[MOD_IF]},
		{ "debug_MOD_INIT", TYPE_INT|TYPE_NULL, &modtolevel[MOD_INIT]},
		{ "debug_MOD_IPC", TYPE_INT|TYPE_NULL, &modtolevel[MOD_IPC]},
		{ "debug_MOD_LIST", TYPE_INT|TYPE_NULL, &modtolevel[MOD_LIST]},
		{ "debug_MOD_NETWORK", TYPE_INT|TYPE_NULL, &modtolevel[MOD_NETWORK]},
		{ "debug_MOD_CONTROLER", TYPE_INT|TYPE_NULL, &modtolevel[MOD_CONTROLER]},
		{ NULL, 0, NULL }
	};

	// Parse command line arguments
	char ch;
	while((ch = getopt(argc, argv, "a:c:d:eE:Df:rnNFhi:l:L:m:M:p:")) != -1){
		switch(ch){	
			case 'a':
				ccontroler_mac_age = atoi(optarg);
				break;
			case 'c':
				cctrlfile = strdup(optarg);
				break;
			case 'd':
				cmodule = strdup(optarg);
				break;
			case 'D':
				cdo_daemonise=0;
				break;
			case 'E':
				ccontroler_endpoint_age = atoi(optarg);
				break;
			case 'F':
				cforward_unknown=1;
				break;
			case 'N':
				cno_endpoint_discard=0;
				break;
			case 'n':
				cdynamic_mac=0;
				break;
			case 'e':
				cdynamic_endpoint=1;
				break;
			case 'r':
				crelay_broadcast=1;
				break;
			case 'f':
				conffile = strdup(optarg);
				break;
			case 'h':
				usage(argv[0]);
				return 0;
				break;
			case 'i':
				cifname = strdup(optarg);
				break;
			case 'l':
				cudpport = atoi(optarg);
				break;
			case 'L':
				if(sscanf(optarg, "%i", &default_log_level) 
				   == 0)
					default_log_level = -1;
				clevel = default_log_level;
				break;
			case 'm':
				cmacaddr = strdup(optarg);
				break;
			case 'M':
				cmtu = atoi(optarg);
				break;
			case 'p':
				cpidfile = strdup(optarg);
				break;
			default:
				usage(argv[0]);
				return 1;
				break;
		}
	}

	/* Check the default log level is sane. */
	if(default_log_level < 0 || default_log_level > 15) {
		default_log_level = 15;
		logger(MOD_INIT, 1, "Default logging level must be a number"
				" between 0 and 15. Giving up.\n");
		return 1;
	}
	
	/* Parse the config file */
	if (conffile != NULL) {
	  	logger(MOD_INIT, 15, "Parsing config file specified on command line\n");
	  	if (parse_config(main_config,conffile)) {
	    		logger(MOD_INIT,1,"Bad Config file %s, giving up\n", conffile);
	    		return 1;
	  	}
	} else {
		logger(MOD_INIT, 15, "About to parse default config file\n");
		if (parse_config(main_config,"/usr/local/etc/etud.conf")) {
			logger(MOD_INIT,1,"Bad Config file, giving up\n");
			return 1;
	  	}
	}
	
	/* Override the config file with any values specified on command line */
	if (cmacaddr != NULL)
		macaddr = strdup(cmacaddr);
	if (cmodule != NULL)
		module = strdup(cmodule);
	if (cpidfile != NULL) 
		pidfile = strdup(cpidfile);
	if (cdo_daemonise == 0)
		do_daemonise = 0;
	if (cforward_unknown == 1)
		forward_unknown = 1;
	if (crelay_broadcast == 1)
		relay_broadcast = 1;
	if (cdynamic_mac == 0)
		dynamic_mac = 0;
	if (cno_endpoint_discard == 0)
		no_endpoint_discard = 0;
	if (cdynamic_endpoint == 1)
		dynamic_endpoint = 1;
	if (cudpport != -1)
		udpport = cudpport;
	if (cmtu != -1)
		mtu = cmtu;
	if (ccontroler_mac_age != -1)
		controler_mac_age = ccontroler_mac_age;
	if (ccontroler_endpoint_age != -1)
		controler_endpoint_age = ccontroler_endpoint_age;
	if (cctrlfile != NULL) 
		ctrlfile = strdup(cctrlfile);
	if (cifname != NULL)
		ifname = strdup(cifname);
	if (clevel != -1)
		default_log_level=clevel;
		

	/* Setup the bridge timer if we are a controler */
	if (dynamic_mac) {
		init_controler();
	}


	/* Check that ifname is set */
	if (ifname == NULL) {
#ifdef LINUX
		ifname = strdup("wan0");
#else
		ifname = strdup("tap0");
#endif
	}
	/* Check that a control file has been specified */
	if (ctrlfile == NULL) {
		sprintf(buf, "/var/run/Etud.%s.ctrl", ifname);
		ctrlfile = strdup(buf);
	}
	
	logger(MOD_INIT, 15, "Parsed config, about to load driver\n");
	if (!load_module(module)) {
		logger(MOD_INIT, 1, "Failed to load driver.\n");
		logger(MOD_INIT, 1, "Aborting...\n");
		return 1;
	}
	
	logger(MOD_INIT, 15, "Loaded driver, about to init interface\n");
	if (!init_interface()) {
		logger(MOD_INIT, 1, "Failed to initialise interface.\n");
		logger(MOD_INIT, 1, "Aborting...\n");
		return 1;
	}

	logger(MOD_INIT, 15, "Initialised interface, about to start UDP\n");
	if (udp_start()<0) {
		logger(MOD_INIT, 1, "Failed to create udp socket.\n");
		logger(MOD_INIT, 1, "Aborting...\n");
		shutdown_interface();
		return 1;
	}

	logger(MOD_INIT, 15, "UDP started, about to start UNIX domain socket\n");
	if (ui_setup()<0) {
		logger(MOD_INIT, 1, "Failed to create unix domain socket.\n");
		logger(MOD_INIT, 1, "Aborting...\n");
		shutdown_interface();
		return 1;
	}

	logger(MOD_INIT, 6, "Etud started\n");
	logger(MOD_INIT, 6, "Using interface driver: %s\n", driver->name);
	logger(MOD_INIT, 6, " version: %s\n", driver->version);

	if (do_daemonise) {
		/* Lets go to the background */
		logger(MOD_INIT, 7, "Attempting to Daemonise...\n");

		daemonise(argv[0]);
		put_pid(pidfile);
		logger(MOD_INIT, 7, "Daemonised\n");
	}

	logger(MOD_INIT, 8, "Init complete\n");
	mainloop();
 	// Clean up the control file
	unlink("/var/run/Etud.ctrl");
      	// Clean up the pid file
      	if(daemonised){
            unlink(pidfile);
      	}
      	// shutdown the interface
	logger(MOD_INIT, 8, "Attempting to shutdown interface\n");
	shutdown_interface();
}
