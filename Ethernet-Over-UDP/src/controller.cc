
#include <sys/time.h> /* for timeval, select timeouts */
#include <signal.h> /* for sigaction (call and struct) */
#include <stdio.h> /* for fprintf and perror */
#include <errno.h>
#include "debug.h"
#include "controler.h"
#include <map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "udp.h"



using namespace std;


void age_bridge( int signo ) 
{
  timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);

  bridge_table_t::iterator it;
  bridge_table_t::iterator del =  bridge_table.end();
  for(it=bridge_table.begin(); it!= bridge_table.end(); it++){
	  if (del != bridge_table.end()){
		  bridge_table.erase(del);
		  del = bridge_table.end();
	  }
	  if (it->second.ts.tv_sec != 0 && it->second.ts.tv_sec < tp.tv_sec){
		  logger(MOD_CONTROLER, 6, "Expire mac(%s, %s:%d)\n", it->first(), inet_ntoa(it->second.addr.sin_addr), ntohs(it->second.addr.sin_port));
		  del = it;
	  }
  }
  if (del != bridge_table.end()){
	  bridge_table.erase(del);
	  del = bridge_table.end();
  }

  endpoint_t::iterator it2;
  endpoint_t::iterator del2 =  endpoint_table.end();
  for(it2=endpoint_table.begin(); it2!= endpoint_table.end(); it2++){
	  if (del2 != endpoint_table.end()){
		  endpoint_table.erase(del2);
		  del2 = endpoint_table.end();
	  }
	  if (it2->second.tv_sec < tp.tv_sec){
		  logger(MOD_CONTROLER, 5, "Expire endpoint (%s:%d)\n", inet_ntoa(it2->first.sin_addr), ntohs(it2->first.sin_port));
		  del2 = it2;
	  }
  }
  if (del2 != endpoint_table.end()){
	  endpoint_table.erase(del2);
	  del2 = endpoint_table.end();
  }
}

int add_age_timer( void )
{
	struct sigaction handler;
  
	handler.sa_handler = &age_bridge;
	handler.sa_flags = SA_RESTART;

	sigemptyset(&handler.sa_mask);
	if (sigaction(SIGALRM, &handler, NULL) < 0) {
	  	logger(MOD_INIT, 3 , "Failed to add signal handler:"
			" SIGALRM, %s\n", strerror(errno));
	  	return -1;
	}
	
	return 0;
}

void init_controler() {
  	logger(MOD_INIT, 3 , "Foo, %s\n", strerror(errno));
    struct itimerval itimer;
	add_age_timer();
	itimer.it_interval.tv_sec=10;
	itimer.it_interval.tv_usec=0;
	itimer.it_value.tv_sec=10;
	itimer.it_value.tv_usec=0;
	if (setitimer(ITIMER_REAL, &itimer, NULL) == -1){
	  	logger(MOD_INIT, 3 , "Failed to set timer, %s\n", strerror(errno));
	}
}



void learn_mac(ether_t mac, sockaddr_in addr, timespec* tp){
  bridge_table_t::iterator it = bridge_table.find(mac);

  if (it == bridge_table.end()){
		bridge_entry be;
		if (tp == NULL){
			tp = new timespec;
			clock_gettime(CLOCK_MONOTONIC, tp);
		}
		be.addr = addr;
		be.ts.tv_sec = tp->tv_sec + controler_mac_age;
		be.ts.tv_nsec = tp->tv_nsec;
		bridge_table[mac] = be;
	  	logger(MOD_CONTROLER, 6, "Learn mac(%s, %s:%d)\n", mac(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
  }else{
		it->second.ts.tv_sec = tp->tv_sec;
		it->second.ts.tv_nsec = tp->tv_nsec;

		// Allow for the MAC to move tunnels
		if (!(addr == (it->second).addr)){
			it->second.addr = addr;
		  	logger(MOD_CONTROLER, 6, "Relearn mac(%s, %s:%d)\n", mac(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		}
  }

}

void learn_endpoint( sockaddr_in addr, timespec* tp){
  timespec t;
  endpoint_t::iterator it = endpoint_table.find(addr);

	if (it == endpoint_table.end()){
		if (tp == NULL){
			tp = new timespec;
			clock_gettime(CLOCK_MONOTONIC, tp);
		}
	  	logger(MOD_CONTROLER, 5, "Learn endpoint (%s:%d)\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		t = *tp;
		t.tv_sec += controler_mac_age;
		endpoint_table[addr] = t;
	}else
		it->second.tv_sec = tp->tv_sec;
		it->second.tv_nsec = tp->tv_nsec;
}
