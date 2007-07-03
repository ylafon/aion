/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#ifndef _AION_ACTION_H_
#define _AION_ACTION_H_

#include "defs.h"
#include "types.h"

void Pong PARAM2(irc_bot *, char *);
void Join PARAM2(irc_bot *, char *);
void Notice PARAM4(irc_bot *, char *, char *, char *);
void PrivMsg PARAM3(irc_bot *, char *, char *);
void Mode PARAM4(irc_bot *, char *, char *,char *);
void Kick PARAM4(irc_bot *, char *, char *, char *);
char *Ban PARAM3(irc_bot *, char *, char *);
char *BanDop PARAM4(irc_bot *, char *, char *, char *);

#endif /* _AION_ACTION_H_ */
