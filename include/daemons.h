/* Wand Project - Ethernet Over UDP
 * $Id: daemons.h,v 1.4 2002/11/30 10:07:50 jimmyish Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */
    
#ifndef DAEMONS_H
#define DAEMONS_H
    
#ifdef __cplusplus
extern "C" {
#endif


void put_pid( char *fname );
void daemonise( char *name ) ;

extern int daemonised;

#ifdef __cplusplus
}
#endif

#endif /* DEAMONS_H */
    
