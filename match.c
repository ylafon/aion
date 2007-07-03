/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: match.c,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "match.h"

int Match(str1,str2)      /* it is ugly... but it works !!! */
    char *str1,*str2;
{
    char *dump1,*dump2,*save1,*save2;
    register int ok,loc_ok,i,len1,len2;
    
    if (!*str1 || !*str2) /* NULL string return(1) FindBan("*"..) */
	return(1);   
    
    save1=(char *)malloc(strlen(str1)+1);
    save2=(char *)malloc(strlen(str2)+1);
 
    dump1 = str1;
    dump2 = save1;

    while (*dump1)
	*dump2++ = ((*dump1>96)&&(*dump1<123)) ? ((*dump1++)-32) : *dump1++;
    *dump2=0;

    dump1 = str2;
    dump2 = save2;

    while (*dump1)
	*dump2++ = ((*dump1>96)&&(*dump1<123)) ? ((*dump1++)-32) : *dump1++;
    *dump2=0;

    dump1 = save1;
    dump2 = save2;

    loc_ok=1;
    ok=0;
    
    
    while (loc_ok&&!ok&&*dump1&&*dump2) {
	if ((*dump1=='*')||(*dump2=='*')) {/* star case (to heaven) arf! */
	    if (*dump1=='*') {
		dump1++;
		if (*dump1) {
		    len2=strlen(dump2);
		    for (i=0;!ok&&i<len2;i++)
			ok+=Match(dump1,dump2+i);
		} else
		    ok=1; /* * at the end is ok */
	    } else {
		dump2++;
		if (*dump2) {
		    len1=strlen(dump1);
		    for (i=0;!ok&&i<len1;i++)
			ok+=Match(dump1+i,dump2);
		} else {
		    ok=1;
		}
	    }
	} else {
	    if ((*dump1=='?')||(*dump2=='?')) {
		dump1++;
		dump2++;
	    } else {
		if ((*dump1=='@')||(*dump2=='@')) { /* @ = [0-9] (here) */
		    if (*dump1=='@') {
			if (((*dump2>='0')&&(*dump2<='9'))||(*dump2=='@')) {
			    dump1++;
			    dump2++;
			} else
			    loc_ok=0;  /* loc_ok is here to avoid checking after a failure */
		    } else {
			if (((*dump1>='0')&&(*dump1<='9'))||(*dump1=='@')) {
			    dump1++;
			    dump2++;
			} else
			    loc_ok=0; /* so... it is supposed to be quite fast */
		    }
		} else {   /* not * ? or @ so... it must be a character */
		    loc_ok=(*dump1==*dump2);
		    dump1++;
		    dump2++;
		}
	    }
	}
    } /* here... ok=1 ->good , loc_ok=0 ->bad , at the end... must check */
  
    if (!ok&&loc_ok) {
	/* loc_ok && !ok means 'at the end' */
	while ((*dump1=='*')||(*dump1=='@')) dump1++;
	while ((*dump2=='*')||(*dump2=='@')) dump2++;
	ok=!*dump1&&!*dump2;
    }
    /* ok=1, nothing more... loc_ok=0, don't continue so... */
    free(save1);
    free(save2);
    return(ok);
}

int MatchAddr(addr1,addr2)
    char *addr1,*addr2;
{
    int ok;
    char login1[32],login2[32];
    register char *tmp1,*tmp2,*tmp;
    
    if (!*addr1 || !*addr2)
	return(0);
    ok=0;
    
    tmp = login1;
    tmp1 = addr1;
    while (*tmp1&&(*tmp1!='@'))
	*tmp++=*tmp1++;
    *tmp=0;
    
    if (!*tmp1)
	return(0); /* end.. no '@', exiting */
	    
    tmp1++;
    tmp = login2;
    tmp2 = addr2;

    while (*tmp2&&(*tmp2!='@'))
	*tmp++=*tmp2++;
    *tmp=0;
    
    if (!*tmp2)
	return(0); /* end.. no '@', exiting */
	
    tmp2++;

    ok = (Match(login1,login2)) ? Match(tmp1,tmp2) : 0;

    return(ok);
}
