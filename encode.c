/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"

void Rot13(encoded)
    char *encoded;
{
    char *tmp;
    
    for (tmp = encoded; *tmp && *tmp!='\n'; tmp++) {
	if (*tmp>='a' && *tmp<'n')
	    *tmp += 13;
	else if (*tmp>'m' && *tmp<='z')
		*tmp -= 13;
	else if (*tmp>='A' && *tmp<'N')
		    *tmp += 13;
	else if (*tmp>'M' && *tmp<='Z')
	    *tmp -= 13;
    }
}

void Decode(encoded)
    char *encoded;
{
    char *tmp1, *tmp2;
    
    for (tmp1 = tmp2 = encoded; *tmp1 && *tmp1!='\n'; tmp1++) {
	*tmp2 = (*tmp1++ - 0x41) << 4;
	if (*tmp1 && *tmp1!='\n')
	    *tmp2++ |= *tmp1 - 0x41; 
    }
    if (tmp1 != tmp2)
	*tmp2 = 0;
}
