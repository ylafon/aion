/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "defs.h"
#include "types.h"
#include "compare.h"
#include "match.h"

int FindUser(cell, testcell)
    void *cell,*testcell;
{
    int ok;

    ok = ((users *)cell)->flags & ((users *)testcell)->flags;
    if (ok) {
	ok = Match(((users *)cell)->channel, ((users *)testcell)->channel);
	if (ok)
	    ok = MatchAddr(((users *)cell)->addr, ((users *)testcell)->addr);
    }
    return(ok);
}

int FindUserStrict(cell, testcell)
    void *cell,*testcell;
{
    int ok;

    ok = ((users *)cell)->flags & ((users *)testcell)->flags;
    if (ok) {
	ok = !strcasecmp(((users *)cell)->channel, ((users *)testcell)->channel);
	if (ok)
	    ok = !strcasecmp(((users *)cell)->addr, ((users *)testcell)->addr);
    }
    return(ok);
}

int FindChannel(cell, testcell)
    void *cell,*testcell;
{
    return (Match(((chans *)cell)->name, ((chans *)testcell)->name));
}

int FindHack(cell, testcell)
    void *cell, *testcell;
{
    int ok;

    ok = Match(((hacks *)cell)->chan, ((hacks *)testcell)->chan);
    if (ok) {
	ok = !strcmp(((hacks *)cell)->nick, ((hacks *)testcell)->nick);
	if (ok)
	    ok = !strcmp(((hacks *)cell)->addr, ((hacks *)testcell)->addr);
    }
    return(ok);
}

int FindWho(cell, testcell)
    void *cell, *testcell;
{
    return( !strcasecmp(((who *)cell)->nick, ((who *)testcell)->nick));
}

int FindWhoChan(cell, testcell)
    void *cell, *testcell;
{
    int ok;
    
    ok = !strcasecmp(((who *)cell)->nick, ((who *)testcell)->nick);
    if (ok)
	ok = Match(((who *)cell)->chan, ((who *)testcell)->chan);
    return(ok);
}

int FindWhoLeave(cell, testcell)
    void *cell, *testcell;
{
    return( Match(((who *)cell)->chan, ((who *)testcell)->chan));
}

int FindTodo(cell, testcell)
    void *cell, *testcell;
{
    return(!strcasecmp(((todos *)cell)->tobedone, ((todos *)testcell)->tobedone));
}

int FindBanUser(cell, testcell)
    void *cell,*testcell;
{
    int ok,after_at;
    char *stripped_addr, *pos, *pos_strip;

    ok = ((users *)cell)->flags & ((users *)testcell)->flags;
    if (ok) {
	ok = Match(((users *)cell)->channel, ((users *)testcell)->channel);
	if (ok) {
	    stripped_addr = (char *)malloc(strlen(((users *)cell)->addr)+1);
	    pos = ((users *)cell)->addr;
	    pos_strip = stripped_addr;
	    after_at = FALSE;
	    while (*pos) {
		if (after_at) {
		    if (*pos != '*')
			*pos_strip++ = *pos++;
		    else
			pos++;
		} else {
		    *pos_strip++ = *pos++;
		    if (*pos == '@')
			after_at = TRUE;
		}
	    }
	    *pos_strip = 0;
	    ok = MatchAddr(stripped_addr, ((users *)testcell)->addr);
	    free(stripped_addr);
	    
	    stripped_addr = (char *)malloc(strlen(((users *)testcell)->addr)+1);
	    pos = ((users *)testcell)->addr;
	    pos_strip = stripped_addr;
	    after_at = FALSE;
	    while (*pos) {
		if (after_at) {	
		    if (*pos != '*')
			*pos_strip++ = *pos++;
		    else
			pos++;
		} else {
		    *pos_strip++ = *pos++;
		    if (*pos == '@')
			after_at = TRUE;
		}
	    }
	    *pos_strip = 0;
	    ok += MatchAddr(stripped_addr, ((users *)cell)->addr);
	    free(stripped_addr);
	}
    }
    return(ok);
}

int FindBan(cell, testcell)
    void *cell,*testcell;
{
    int ok, after_at;
    char *stripped_addr, *pos, *pos_strip;

    ok = Match(((ban *)cell)->chan, ((ban *)testcell)->chan);
    if (ok) {
	stripped_addr = (char *)malloc(strlen(((ban *)cell)->addr)+1);
	pos = ((ban *)cell)->addr;
	pos_strip = stripped_addr;
	after_at = FALSE;
	while (*pos) {
	    if (after_at) {		
		if (*pos != '*')
		    *pos_strip++ = *pos++;
		else
		    pos++;
	    } else {
		*pos_strip++ = *pos++;
		if (*pos == '@')
		    after_at = TRUE;
	    }
	}
	*pos_strip = 0;
	ok = MatchAddr(stripped_addr, ((ban *)testcell)->addr);
	free(stripped_addr);
	
	stripped_addr = (char *)malloc(strlen(((ban *)testcell)->addr)+1);
	pos = ((ban *)testcell)->addr;
	pos_strip = stripped_addr;
	after_at = FALSE;
	while (*pos) {
	    if (after_at) {	
		if (*pos != '*')
		    *pos_strip++ = *pos++;
		else
		    pos++;
	    } else {
		*pos_strip++ = *pos++;
		if (*pos == '@')
		    after_at = TRUE;
	    }
	}
	*pos_strip = 0;
	ok += MatchAddr(stripped_addr, ((ban *)cell)->addr);
	free(stripped_addr);
    }
    return(ok);
}

int FindBanChannel(cell, testcell)
    void *cell, *testcell;
{
    return(Match(((ban *)cell)->chan, ((ban *)testcell)->chan));
}
