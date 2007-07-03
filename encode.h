/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#ifndef _AION_ENCODE_H_
#define _AION_ENCODE_H_

#include "defs.h"
#include "types.h"

#define ROT13_CHAR  '~'
#define ENCODE_CHAR '#'

void Rot13 PARAM1(char *);
void Decode PARAM1(char *);

#endif /* _AION_ENCODE_H */
