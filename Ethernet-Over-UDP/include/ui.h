/* Wand Project - Ethernet Over UDP
 * $Id: ui.h,v 1.3 2002/11/30 05:54:45 gianp Exp $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#ifndef UI_H
#define UI_H

int ui_setup(char *s="/var/run/Etud.ctrl");
int ui_process(int fd);
int ui_process_request(int fd);
int ui_send(int sock,char *msg);

#endif

