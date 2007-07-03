/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: time.h,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

#ifndef _AION_TIME_H_
#define _AION_TIME_H_

#include "defs.h"

#ifdef SUNOS
#include <time.h>
double difftime PARAM2(time_t, time_t);
#endif /* SUNOS */
void GmtTime PARAM1(char *);
void LocalTime PARAM1(char *);
void ShortTime PARAM1(char *);

#endif /* _AION_TIME_H_ */
