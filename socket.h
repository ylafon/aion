/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: socket.h,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

#ifndef _AION_SOCKET_H_
#define _AION_SOCKET_H_

#include "defs.h"

int Connection PARAM2(char *, int);
int SendSocket PARAM2(int ,char *);
int ReadSocket PARAM4(int ,char *, char **, char *);

char *NameToIp PARAM1(char *);
char *IpToName PARAM1(char *);

#endif /* _AION_SOCKET_H_ */
