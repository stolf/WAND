/* Wand Project - Ethernet Over UDP
 * $Id: daemons.h,v 1.3 2002/04/17 12:13:18 jimmyish Exp $
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


#ifdef __cplusplus
}
#endif

#endif /* DEAMONS_H */
    
