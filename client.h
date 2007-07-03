/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#ifndef _AION_CLIENT_H_
#define _AION_CLIENT_H_

#include "defs.h"
#include "types.h"

void ClientParsePublic PARAM5(irc_bot *, char *, char *, char *, char *);
void ClientParsePrivate PARAM4(irc_bot *, char *, char *, char *);
void ClientParseUserhost PARAM2(irc_bot *, char *);
void ClientParseBan PARAM2(irc_bot *, char *);

#endif /* _AION_CLIENT_H_ */
