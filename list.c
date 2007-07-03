/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: list.c,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "types.h"
#include "list.h"
#include "match.h"
#include "cell.h"

/**
 * add a Cell at the beginning of the list
 */
void AddCellFirst( TheList, type, cell)
    list **TheList;
    int type;
    void *cell;
{
    list *new_link;

    new_link = (list *) malloc(sizeof(list));
    new_link->type = type;
    new_link->cell = cell;
    new_link->next = *TheList;
    new_link->prev = NULL;
    if (*TheList)
      (*TheList)->prev = new_link;
    *TheList = new_link;
}

/**
 * add a Cell at the end of the list
 */
void AddCellLast( TheList, type, cell)
    list **TheList;
    int type;
    void *cell;
{
    list *new_link,*last_link;
    
    new_link = (list *) malloc(sizeof(list));
    new_link->type = type;
    new_link->cell = cell;
    new_link->next = NULL;
    if (*TheList) {
	last_link = *TheList;
	while (last_link->next)
	    last_link = last_link->next;
	
	last_link->next = new_link;
	new_link->prev = last_link;
    } else {
	new_link->prev = NULL;
	*TheList = new_link;
    }
}

/**
 * find the item in the list containing the matching cell
 */
list *FindCell(TheList, cell, compare_func)
    list **TheList;
    void *cell;
    int (*compare_func) PARAM2(void *, void *);
{
    int ok;
    list *find_link,*found_link;

    ok = 0;
    found_link = NULL;

    for (find_link = *TheList; !ok && find_link; find_link = find_link->next) {
	ok = compare_func(cell, find_link->cell);
	if (ok)
	    found_link = find_link;
    }

    return(found_link);
}

/**
 * find the address of the item in the list containing the matching cell
 */
list **FindCellAddr(TheList, cell, compare_func)
    list **TheList;
    void *cell;
    int (*compare_func) PARAM2(void *, void *);
{
    int ok;
    list **find_link,**found_link;

    ok = 0;
    found_link = NULL;

    for (find_link = TheList; !ok && *find_link; 
	 find_link = &((*find_link)->next)) {
	ok = compare_func(cell, (*find_link)->cell);
	if (ok)
	    found_link = find_link;
    }
    
    return(found_link);
}

/**
 * free a list item
 */
void FreeLink(the_link)
    list **the_link;
{
    list *TheLink;

    if (!the_link)
	return;
    TheLink = *the_link;
    if (TheLink) {
	if (TheLink->next)
	    TheLink->next->prev = TheLink->prev;
	if (TheLink->prev)
	    TheLink->prev->next = TheLink->next;
	else
	    *the_link = TheLink->next;
	FreeCell(TheLink->type,TheLink->cell);
	free(TheLink);
    }
}

/**
 * "eat" a list item, but don't free it
 */
void SwallowLink(TheLink)
    list *TheLink;
{
    if (TheLink->next)
	TheLink->next->prev = TheLink->prev;
    if (TheLink->prev)
	TheLink->prev->next = TheLink->next;
}

/**
 * free the whole list
 */
void FreeList(TheList)
    list **TheList;
{
    list *tmp_link,*next_link;
    
    next_link = *TheList;
    while (next_link) {
	tmp_link = next_link;
	next_link = next_link->next;
	FreeCell(tmp_link->type, tmp_link->cell);
	free(tmp_link);
    }
    *TheList = NULL;
}

/**
 * put the first element of the list in the last position
 */
void LoopList(TheList)
    list **TheList;
{
    list *last_link,*new_list;

    if (*TheList) {
	if ((*TheList)->next) {
	    new_list = (*TheList)->next;
	    last_link = *TheList;
	    
	    while (last_link->next)
		last_link = last_link->next;
	    
	    last_link->next = *TheList;
	    (*TheList)->next = NULL;
	    (*TheList)->prev = last_link;
	    
	    new_list->prev = NULL;
	    *TheList = new_list;
	}
    }
}
