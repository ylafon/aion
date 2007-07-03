/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include "defs.h"
#include "types.h"
#include "cell.h"
#include "list.h"
#include "compare.h"
#include "socket.h"
#include "bot.h"

void FreeCell(type, cell)
    int type;
    void *cell;
{
    if (!cell)
	return;
    switch(type) {
    case CELL_HOSTS:
	free(((hosts *)cell)->name);
	break;
    case CELL_NICKS:
	free(((nicks *)cell)->nick);
	break;
    case CELL_USERS:
	if (((users *)cell)->addr)
	    free(((users *)cell)->addr);
	if (((users *)cell)->channel)
	    free(((users *)cell)->channel);
	break;
    case CELL_CHANS:
	free(((chans *)cell)->name);
	if (((chans *)cell)->passwd)
	    free(((chans *)cell)->passwd);
	if (((chans *)cell)->topic)
	    free(((chans *)cell)->topic);
	break;
    case CELL_HACKS:
	if (((hacks *)cell)->chan)
	    free(((hacks *)cell)->chan);
	if (((hacks *)cell)->nick)
	    free(((hacks *)cell)->nick);
	if (((hacks *)cell)->addr)
	    free(((hacks *)cell)->addr);
	break;
    case CELL_WHO:
	if (((who *)cell)->chan)
	    free(((who *)cell)->chan);
	if (((who *)cell)->nick)
	    free(((who *)cell)->nick);
	if (((who *)cell)->addr)
	    free(((who *)cell)->addr);
	break;
    case CELL_BOT:
	FreeBot((irc_bot *)cell);
	break;
    case CELL_TODOS:
	free(((todos *)cell)->tobedone);
	break;
    case CELL_UH:
	if (((uh *)cell)->chan)
	    free(((uh *)cell)->chan);
	if (((uh *)cell)->nick)
	     free(((uh *)cell)->nick);
	break;
    case CELL_BAN:
	if (((ban *)cell)->chan)
	    free(((ban *)cell)->chan);
	if (((ban *)cell)->addr)
	    free(((ban *)cell)->addr);
	break;
    default:
	printf("**** PANIC! **** UNKNOWN CELL TYPE IN FreeCell\n");
	break;
    }
    free(cell);
}

chans *CreateChannel(name, flags, passwd, size, mode)
    char *name,*passwd;
    int flags,size,mode;
{
    chans *new_channel;

    new_channel = (chans *)malloc(sizeof(chans));
    new_channel->name = (char *)malloc(strlen(name)+1);
    strcpy(new_channel->name, name);
    new_channel->flags = flags;
    new_channel->mode = mode;
    if (passwd) {
	new_channel->passwd = (char *)malloc(strlen(passwd)+1);
	strcpy(new_channel->passwd, passwd);
	new_channel->mode |= PLUS_K;
    } else
	new_channel->passwd = NULL;
    new_channel->size = size;
    if (size)
	new_channel->mode |= PLUS_L;
    new_channel->uptime = time((time_t *)NULL);
    new_channel->last_mode = new_channel->uptime;
    new_channel->iamchanop = FALSE;
    new_channel->topic = (char *)NULL;
    return(new_channel);
}

who *CreateWho(nick, addr, channel)
    char *nick,*addr,*channel;
{
    who *new_who;

    new_who = (who *)malloc(sizeof(who));

    if (nick) {
	new_who->nick = (char *)malloc(strlen(nick)+1);
	strcpy(new_who->nick,nick);
    } else
	new_who->nick = NULL;
    if (addr) {
	new_who->addr = (char *)malloc(strlen(addr)+1);
	strcpy(new_who->addr,addr);
    }
    else
	new_who->addr = NULL;
    if (channel) {
	new_who->chan = (char *)malloc(strlen(channel)+1);
	strcpy(new_who->chan,channel);
    } else
	new_who->chan = NULL;
    new_who->uptime = time((time_t *)NULL);
    new_who->chanop = 0;
    return(new_who);
}

hacks *CreateHack(chan ,nick, addr)
    char *nick, *addr, *chan;
{
    hacks *new_hack;

#ifdef DEBUG
    printf("CreateHack: chan [%s], nick [%s], addr [%s]\n", chan, nick, addr);
#endif

    new_hack = (hacks *) malloc(sizeof(hacks));

    new_hack->chan = (char *)malloc(strlen(chan)+1);
    strcpy(new_hack->chan, chan);
    new_hack->nick = (char *)malloc(strlen(nick)+1);
    strcpy(new_hack->nick, nick);
    new_hack->addr = (char *)malloc(strlen(addr)+1);
    strcpy(new_hack->addr, addr);
    new_hack->uptime = time((time_t *)NULL);
    return (new_hack);
}

todos *CreateTodo(tobedone, todo_time)
    char *tobedone;
    int todo_time;
{
    todos *newtodo;

    newtodo = (todos *)malloc(sizeof(todos));

    newtodo->tobedone = (char *)malloc(strlen(tobedone)+1);
    strcpy(newtodo->tobedone, tobedone);
    newtodo->todo_time = todo_time;
    newtodo->when = time((time_t *)NULL);
    return (newtodo);
}

users *CreateUser(addr, channel, mask)
    char *addr,*channel;
    int mask;
{
    users *newuser;

    newuser = (users *)malloc(sizeof(users));

    newuser->addr = (char *)malloc(strlen(addr)+1);
    strcpy(newuser->addr,addr);
    newuser->channel = (char *)malloc(strlen(channel)+1);
    strcpy(newuser->channel,channel);
    newuser->flags = mask;
    newuser->actions = 0;
    newuser->bad_actions = 0;
    newuser->uptime = time((time_t *)NULL);
    return(newuser);
}

void UpdateUser(TheList, addr, chan, flags, us_time, compare_func)
    list **TheList;
    char *addr,*chan;
    int flags,us_time;
    int (*compare_func) PARAM2(void *, void *);
{
    list *dummylist ;
    users *dummyuser;
    time_t the_time;
    
    dummyuser = CreateUser(addr, chan, US_MASTER|US_FRIEND|US_USER|US_BOT|US_AUTOOP|US_KPRO|US_OPRO|US_BPRO|US_SHIT|US_REBAN);
    dummylist = FindCell(TheList, dummyuser, compare_func);
    FreeCell(CELL_USERS, dummyuser);
    dummyuser = (dummylist) ? ((users *)dummylist->cell) : CreateUser(addr, chan, flags);
    the_time = time((time_t *)NULL);
    if (flags & US_MASTER) {
	dummyuser->ms_time = us_time;
	dummyuser->ms_when = the_time;
    }
    if (flags & US_FRIEND) {
	dummyuser->fr_time = us_time;
	dummyuser->fr_when = the_time;
    }
    if (flags & US_USER) {
	dummyuser->us_time = us_time;
	dummyuser->us_when = the_time;
    }
    if (flags & US_BOT) {
	dummyuser->bot_time = us_time;
	dummyuser->bot_when = the_time;
    }
    if (flags & US_AUTOOP) {
	dummyuser->op_time = us_time;
	dummyuser->op_when = the_time;
    }
    if (flags & US_KPRO) {
	dummyuser->kpro_time = us_time;
	dummyuser->kpro_when = the_time;
    }
    if (flags & US_OPRO) {
	dummyuser->opro_time = us_time;
	dummyuser->opro_when = the_time;
    }
    if (flags & US_BPRO) {
	dummyuser->bpro_time = us_time;
	dummyuser->bpro_when = the_time;
    }
    if (flags & US_SHIT) {
	dummyuser->sh_time = us_time;
	dummyuser->sh_when = the_time;
    }
    if (flags & US_REBAN) {
	dummyuser->reban_time = us_time;
	dummyuser->reban_when = the_time;
    }
    dummyuser->flags |=flags;
    if (!dummylist) {
	AddCellFirst(TheList, CELL_USERS, dummyuser);
	printf("Adding User %s on channel %s for %ds, mode %d\n",
	       dummyuser->addr, dummyuser->channel, us_time, flags);
    }
}

nicks *CreateNick(nick)
    char *nick;
{
    nicks *newnick;
    
    newnick = (nicks *)malloc(sizeof(nicks));
    newnick->nick = (char *)malloc(strlen(nick)+1);
    strcpy(newnick->nick, nick);
    newnick->uptime = (time_t) 0;
    return(newnick);
}

hosts *CreateHost(host, port)
    char *host;
    int  port;
{
    hosts *newhost;
    
    newhost = (hosts *)malloc(sizeof(hosts));
    newhost->name = (char *)malloc(strlen(host)+1);
    strcpy(newhost->name,host);
    newhost->port = port;
    newhost->passwd = NULL;
    newhost->uptime = (time_t) 0;
    return(newhost);
}

hosts *CreateHostPass(host, port, pass)
    char *host;
    int  port;
    char *pass;
{
    hosts *newhost;
    
    newhost = (hosts *)malloc(sizeof(hosts));
    newhost->name = (char *)malloc(strlen(host)+1);
    strcpy(newhost->name,host);
    newhost->port = port;
    newhost->passwd = (char *)malloc(strlen(pass)+1);
    strcpy(newhost->passwd, pass);
    newhost->uptime = (time_t) 0;
    return(newhost);
}


uh *CreateUh(type, nick, chan, timer)
    int type, timer;
    char *nick, *chan;
{
    uh *new_uh;

    new_uh = (uh *)malloc(sizeof(uh));
    new_uh->type = type;
    new_uh->timer = timer;
    if (nick) {
	new_uh->nick = (char *)malloc(strlen(nick)+1);
	strcpy(new_uh->nick, nick);
    } else
	new_uh->nick = NULL;
    if (chan) {
	new_uh->chan = (char *)malloc(strlen(chan)+1);
	strcpy(new_uh->chan, chan);
    } else
	new_uh->chan = NULL;
    return(new_uh);
}

ban *CreateBan(chan, addr)
    char *chan, *addr;
{
    ban *new_ban;

    new_ban = (ban *)malloc(sizeof(ban));
    if (addr) {
	new_ban->addr = (char *)malloc(strlen(addr)+1);
	strcpy(new_ban->addr, addr);
    } else
	new_ban->addr = NULL;
    new_ban->chan = (char *)malloc(strlen(chan)+1);
    strcpy(new_ban->chan, chan);
    return(new_ban);
}

void PurgeUserList(TheList)
    list **TheList;
{
    list *tmp_link, *next_link;
    users *the_user;
    time_t the_time;
    int is_first;

    the_time = time((time_t *)NULL);
    is_first = TRUE;

    for (tmp_link = *TheList; tmp_link ;) {
	the_user = (users *)tmp_link->cell;
	if (the_user->flags & US_MASTER)
	    if (difftime(the_time, the_user->ms_when)>the_user->ms_time)
		the_user->flags &= ~US_MASTER;
	if (the_user->flags & US_FRIEND)
	    if (difftime(the_time, the_user->fr_when)>the_user->fr_time)
		the_user->flags &= ~US_FRIEND;
	if (the_user->flags & US_USER)
	    if (difftime(the_time, the_user->us_when)>the_user->us_time)
		the_user->flags &= ~US_USER;
	if (the_user->flags & US_BOT)
	    if (difftime(the_time, the_user->bot_when)>the_user->bot_time)
		the_user->flags &= ~US_BOT;
	if (the_user->flags & US_AUTOOP)
	    if (difftime(the_time, the_user->op_when)>the_user->op_time)
		the_user->flags &= ~US_AUTOOP;
	if (the_user->flags & US_KPRO)
	    if (difftime(the_time, the_user->kpro_when)>the_user->kpro_time)
		the_user->flags &= ~US_KPRO;
	if (the_user->flags & US_OPRO)
	    if (difftime(the_time, the_user->opro_when)>the_user->opro_time)
		the_user->flags &= ~US_OPRO;
	if (the_user->flags & US_BPRO)
	    if (difftime(the_time, the_user->bpro_when)>the_user->bpro_time)
		the_user->flags &= ~US_BPRO;
	if (the_user->flags & US_SHIT) 
	    if (difftime(the_time, the_user->sh_when)>the_user->sh_time)
		the_user->flags &= ~US_SHIT;
	if (the_user->flags & US_REBAN)
	    if (difftime(the_time, the_user->reban_when)>the_user->reban_time)
		the_user->flags &= ~US_REBAN;
	if (!the_user->flags) {
	    next_link = tmp_link->next;
	    SwallowLink(tmp_link);
	    FreeCell(CELL_USERS, the_user);
	    if (is_first && !next_link)
		*TheList = NULL;
	    else
		if (is_first)
		    *TheList = next_link;
	    free(tmp_link);
	    tmp_link = next_link;
	} else {
	    tmp_link = tmp_link->next;
	    is_first = FALSE;
	}
    }
}

void PurgeTodoList(bot, TheList)
    irc_bot *bot;
    list **TheList;
{
    list   *tmp_link, *next_link;
    time_t the_time;
    todos    *the_todo;
    int is_first;

    the_time = time((time_t *)NULL);
    is_first = TRUE;
    for (tmp_link = *TheList; tmp_link ;) {
	the_todo = (todos *)tmp_link->cell;
	if (difftime(the_time, the_todo->when)>the_todo->todo_time) {
	    SendSocket(bot->socknum, the_todo->tobedone);
	    next_link = tmp_link->next;
	    SwallowLink(tmp_link);
	    FreeCell(CELL_TODOS, the_todo);
	    if (is_first && !next_link)
		*TheList = NULL;
	    else
		if (is_first)
		    *TheList = next_link;
	    free(tmp_link);
	    tmp_link = next_link;
	} else {
	    is_first = FALSE;
	    tmp_link = tmp_link->next;
	}
    }
}

void PurgeHackList(bot, TheList)
    irc_bot *bot;
    list **TheList;
{
    list *tmp_link, *next_link;
    hacks *the_hack;
    time_t the_time;
    int is_first;

    the_time = time((time_t *)NULL);
    is_first = TRUE;

    for (tmp_link = *TheList; tmp_link ;) {
	the_hack = (hacks *)tmp_link->cell;
	if (difftime(the_time, the_hack->uptime)>bot->hack_max_time) {
	    next_link = tmp_link->next;
	    SwallowLink(tmp_link);
	    FreeCell(CELL_HACKS, the_hack);
	    if (is_first && !next_link)
		*TheList = NULL;
	    else
		if (is_first)
		    *TheList = next_link;
	    free(tmp_link);
	    tmp_link = next_link;
	} else {
	    tmp_link = tmp_link->next;
	    is_first = FALSE;
	}
    }
}
