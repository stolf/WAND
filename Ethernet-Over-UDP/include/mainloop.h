/* Wand Project - Ethernet Over UDP
 * $Id: mainloop.h,v 1.2 2001/08/12 06:00:27 gsharp Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#ifndef MAINLOOP_H
#define MAINLOOP_H

typedef void (*callback_t)(int fd);
void addRead(int fd,callback_t callback);
void remRead(int fd);
void mainloop(void);

#endif