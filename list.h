/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: list.h,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

#ifndef _AION_LIST_H_
#define _AION_LIST_H_

#include "defs.h"
#include "types.h"

void AddCellFirst PARAM3(list **, int, void *);
void AddCellLast PARAM3(list **, int, void *);
list *FindCell PARAM3(list **, void *, int PARAM2(void *, void *) ); 
list **FindCellAddr PARAM3(list **, void *, int PARAM2(void *, void *) ); 
list *FindCell ();
list **FindCellAddr ();
void FreeLink PARAM1(list **);
void SwallowLink PARAM1(list *);
void FreeList PARAM1(list **);
void LoopList PARAM1(list **);

#endif /* _AION_LIST_H_ */
