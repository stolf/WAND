/* Wand Project - Ethernet Over UDP
 * $Id: mainloop.h,v 1.5 2002/11/30 06:13:57 jimmyish Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#ifndef MAINLOOP_H
#define MAINLOOP_H

typedef void (*callback_t)(int fd);
void addRead(int fd,callback_t callback);
void remRead(int fd);
void mainloop(void);
void sig_hnd( int signo );

#endif
