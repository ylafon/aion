/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: strip.c,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "defs.h"
#include "strip.h"
#include "match.h"

int IsValidName(addr)
    char *addr;
{
    int is_ok;
    char *tmp;

    is_ok = TRUE;

    for (tmp=addr;is_ok && *tmp; tmp++)
	is_ok = (*tmp>='0' && *tmp<= '9') || (*tmp>='a' && *tmp<= 'z') ||
	        (*tmp>='A' && *tmp<= 'Z') || (*tmp == '.') || (*tmp == '-');

    return is_ok;
}

int IsIp(addr)
    char *addr;
{
    int is_ok, nb_dot;
    char *tmp;

    is_ok = TRUE;
    nb_dot = 0;

    for (tmp=addr;is_ok && *tmp; tmp++) {
	is_ok = ((*tmp>='0') && (*tmp<= '9')) || (*tmp == '.');
	nb_dot += (*tmp == '.');
    }
    is_ok *= (nb_dot == 3);
    return is_ok;
}
    

int IsNumeric(raw_addr)
    char *raw_addr;
{
    int num_ok;
    int is_really_num;
    char *tmp;
    
    num_ok = TRUE;
    is_really_num = FALSE;
    tmp = raw_addr;
    while (*tmp && (*tmp!='@')) /* skip login */
	tmp++;
    for (tmp++;num_ok && *tmp; tmp++) {
	num_ok=((*tmp>47)&&(*tmp<58))||*tmp=='*'||*tmp=='.';
	is_really_num +=((*tmp>47)&&(*tmp<58));
    }
    return(num_ok&&is_really_num);
}

void Strip(raw_addr, stripped_addr)
    char *raw_addr,*stripped_addr;
{
    char *tmp_raw,*tmp_strip;
    int i;

    tmp_strip = stripped_addr;
    *tmp_strip++='*';
    tmp_raw = raw_addr;
    for (i=0; i<USER_PREFIX_NUM; i++)
	if (*raw_addr == USER_PREFIXES[i])
	    tmp_raw = raw_addr + 1;
    i = 0;
    while (*tmp_raw && *tmp_raw!='@' && (i < MAX_LOGIN_LENGTH)) {
	*tmp_strip++=*tmp_raw++;
	i++;
    }
    if (*tmp_raw!='@' && (i == MAX_LOGIN_LENGTH)) {
	*tmp_strip++='*';
	while (*tmp_raw && *tmp_raw!='@')
	    tmp_raw++;
    }
    *tmp_strip++=*tmp_raw++;
    *tmp_strip++='*';
    while (*tmp_raw && *tmp_raw!='.')
	tmp_raw++;
    while (*tmp_raw)
	*tmp_strip++=*tmp_raw++;
    *tmp_strip=0;
}

void StripNumeric(raw, stripped)
    char *raw, *stripped;
{
    char *tmp_raw,*tmp_strip, *tmp_select;
    int  net_class;
    char ip_select[64];
    int i;

    tmp_strip = stripped;
    *tmp_strip++='*';
    tmp_raw = raw;
    for (i=0; i<USER_PREFIX_NUM; i++)
	if (*raw == USER_PREFIXES[i])
	    tmp_raw = raw + 1;
    i = 0;
    while (*tmp_raw && *tmp_raw!='@' && (i < MAX_LOGIN_LENGTH)) {
	*tmp_strip++=*tmp_raw++;
	i++;
    }
    if (*tmp_raw!='@' && (i == MAX_LOGIN_LENGTH)) {
	*tmp_strip++='*';
	while (*tmp_raw && *tmp_raw!='@')
	    tmp_raw++;
    }

    if (*tmp_raw=='@')
	*tmp_strip++=*tmp_raw++;
    
    tmp_select = ip_select;
    while (*tmp_raw && (*tmp_raw!='.'))
	*tmp_select++ = *tmp_strip++ = *tmp_raw++;

    *tmp_select = 0;
    net_class = atoi(ip_select);

    if (net_class<ENDCLASS_A)
	strcpy(tmp_strip,".*.*.*");
    else
	if (net_class<ENDCLASS_B) {
	    *tmp_strip++ = *tmp_raw++;
	    while (*tmp_raw && (*tmp_raw!='.'))
		*tmp_strip++ = *tmp_raw++;
	    strcpy(tmp_strip,".*.*");
	} else
	    if (net_class<ENDCLASS_C) {
		*tmp_strip++ = *tmp_raw++;
		while (*tmp_raw && (*tmp_raw!='.'))
		    *tmp_strip++ = *tmp_raw++;
		*tmp_strip++ = *tmp_raw++;
		while (*tmp_raw && (*tmp_raw!='.'))
		    *tmp_strip++ = *tmp_raw++;
		strcpy(tmp_strip,".*");
	    }
	    else 
		while (*tmp_raw)
		    *tmp_strip++ = *tmp_raw++;
}
