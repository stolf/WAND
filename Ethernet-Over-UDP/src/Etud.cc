/* Wand Project - Ethernet Over UDP
 * $Id: Etud.cc,v 1.9 2001/08/14 00:58:52 gsharp Exp $
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

int load_module(char *s)
{
	if(!dlopen(s,RTLD_NOW)) {
		cout << "Error loading module '" << s << "': " << dlerror() << endl;
		return 0;
	}
	return 1;
}

int main(int arvc,char **argv)
{
	if (!load_module("drivers/ethertap.so")) {
		cout << "Aborting..." << endl;
		return 1;
	}
	struct interface_t *interface = find_interface("ethertap");
	if ((interface=find_interface("ethertap"))==NULL) {
		cout << "Failed to find driver" << endl;
		cout << "Aborting..." << endl;
		return 1;
	}
	if (!init_interface(interface,1)) {
		cout << "Failed to initialise interface." << endl;
		cout << "Aborting..." << endl;
		return 1;
	}
	if (udp_start()<0) {
		cout << "Failed to create udp socket." << endl;
		cout << "Aborting..." << endl;
		interface->down();
		return 1;
	}
	if (ui_setup()<0) {
		cout << "Failed to create unix domain socket." << endl;
		interface->down();
		return 1;
	}
	cout << "Etud started" << endl;
	cout << "Using interface driver: " << interface->name << endl;
	cout << " version: " << interface->version << endl;

	/* Lets go to the background */
	cout << "Attempting to Daemonise..." << endl;
	daemonise();
	put_pid("Etud");

	mainloop();
}
