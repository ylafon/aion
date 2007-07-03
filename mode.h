/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: mode.h,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

#ifndef _AION_MODE_H_
#define _AION_MODE_H_

#include "defs.h"
#include "types.h"

void ModeAnalysis PARAM5(irc_bot *, char *, char *, char *, char *);
void ForceChanMode PARAM3(irc_bot *, chans *, int);
void ChanModeAnalysis PARAM3(irc_bot *, char *, char *);

#endif /* _AION_MODE_H_ */
