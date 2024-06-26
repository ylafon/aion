/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: strip.h,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

#ifndef _AION_STRIP_H_
#define _AION_STRIP_H_

#include "defs.h"
#include "types.h"

int IsValidName PARAM1(char *);
int IsIp PARAM1(char *);
int IsNumeric PARAM1(char *);
void Strip PARAM2(char *, char *);
void StripNumeric PARAM2(char *, char *);

#endif /* _AION_STRIP_H_ */
