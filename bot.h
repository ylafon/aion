/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#ifndef _AION_BOT_H_
#define _AION_BOT_H_

#include "defs.h"
#include "types.h"

irc_bot *CreateBot PARAM1(void);
void ConnectBot PARAM1(irc_bot *);
void RegisterBot PARAM1(irc_bot *);
void ChangeNick PARAM1(irc_bot *);
void FreeBot PARAM1(irc_bot *);
void ShutBot PARAM1(irc_bot *);

#endif /* _AION_BOT_H */
