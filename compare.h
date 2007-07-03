/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: compare.h,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

#ifndef _AION_COMPARE_H_
#define _AION_COMPARE_H_

#include "defs.h"
#include "types.h"

int FindUser PARAM2(void *, void *);
int FindUserStrict PARAM2(void *, void *);
int FindChannel PARAM2(void *, void *);
int FindHack PARAM2(void *, void *);
int FindWho PARAM2(void *, void *);
int FindWhoChan PARAM2(void *, void *);
int FindWhoLeave PARAM2(void *, void *);
int FindTodo PARAM2(void *, void *);
int FindBanUser PARAM2(void *, void *);
int FindBan PARAM2(void *, void *);
int FindBanChannel PARAM2(void *, void *);

#endif /* _AION_COMPARE_H_ */
