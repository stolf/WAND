/* Wand Project - Ethernet Over UDP
 * $Id: daemons.h,v 1.1 2001/10/27 09:56:24 gsharp Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */
    
#ifndef DAEMONS_H
#define DAEMONS_H
    
#ifdef __cplusplus
extern "C" {
#endif


void put_pid( char *fname );
void daemonise( void ) ;


#ifdef __cplusplus
}
#endif

#endif /* DEAMONS_H */
    