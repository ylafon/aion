/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: parse.c,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/socket.h>
#include "defs.h"
#include "types.h"
#include "list.h"
#include "parse.h"
#include "action.h"
#include "cell.h"
#include "compare.h"
#include "bot.h"
#include "mode.h"
#include "socket.h"
#include "ctcp.h"
#include "strip.h"
#include "match.h"
#include "file.h"
#include "time.h"
#include "encode.h"
#include "debug.h"
#include "socket.h"
#include "client.h"

static void ClientParse PARAM2(irc_bot *, char *);
static void ServerParse PARAM2(irc_bot *, char *);
static void ServiceParse PARAM2(irc_bot *, char *);


void ClientParse(bot, sock_read)
    irc_bot *bot;
    char    *sock_read;
{
    char  *source;
    char  *function;
    char  *msg_to;
    char  *param;
    char  *param1, *param2;
    char  *param3, *param4;
    char  *nick, *addr;
    char  send_socket_buffer[512];
    char  *banned_addr;
    todos *dummytodo;
    hacks *dummyhack;
    who   *dummywho;
    list  *dummylist,**dummylink;
    users *dummyuser, *the_user;
    chans *dummychan;

#ifdef DEBUG
    printf("[%s]\n", sock_read);
#endif

    bot->s_ping = time((time_t *)NULL);
    source = sock_read + (*sock_read == ':');
    nick = source;
    addr = source;

    function = source;
  
    while ((*function!=' ') && (*function!='\n') && *function) {
	if (*function == '!') {
	    *function = 0;
	    addr = function+1;
	}
	function++;
    }
    if (*function)
	*function++=0;
    if (!strcmp(source,"PING")) {
	Pong(bot, function);
	return;
    }

    msg_to = function;
    while ((*msg_to!=' ') && (*msg_to!='\n') && *msg_to)
	msg_to++;
    if (*msg_to)
	*msg_to++=0;
    if (*msg_to == ':')
	msg_to++;
    
    param = msg_to;
    while ((*param!=' ') && (*param!='\n') && *param) {
	if (*param == '\a')
	    *param = 0;
	param++;
    }
    if (*param)
	*param++=0;

    if (!strcmp(source,"NOTICE")) /* ignore server notices */
	return;

    if (!strcmp(source,"ERROR")) {
	char the_time[64];
	
	close(bot->socknum);
	bot->state = STATE_NOT_CONNECTED;
	FreeList(&bot->who_list);
	ShortTime(the_time);
	fprintf(bot->outfile, "*** SERVER ERROR [%s] %s %s: %s\n", the_time, addr,
		((hosts *)bot->host_list->cell)->name, param);
	fflush(bot->outfile);
	if (bot->behaviour & B_LOOP_ON_ERROR)
	    LoopList(&bot->host_list);
	return;
    }

    if (!strcmp(function,"MODE")) {
	ModeAnalysis(bot, msg_to, param, nick, addr);
	return;
    }
	
    if (!strcmp(function,"324")) { /* chan mode reply */
	param1 = param;
	while ((*param1!=' ') && (*param1!='\n') && *param1)
	    param1++;
	if (*param1)
	    *param1++=0;
	ChanModeAnalysis(bot, param, param1);
	return;
    }
	
    if (!strcmp(function,"PRIVMSG")) {
	if ((*msg_to == '#') || (*msg_to == '&'))
	    ClientParsePublic(bot, nick, addr, msg_to, param);
	else
	    ClientParsePrivate(bot, nick, addr, param);
	return;
    }

    if (!strcmp(function,"QUIT")) {
	param1 = msg_to; /* only for clarity... remove this for optimization */
	param2 = param;
	if (strchr(param1,'.') && strchr(param2,'.')) {
	    dummywho = CreateWho(nick, NULL, NULL);
	    dummylink = FindCellAddr(&bot->who_list, dummywho, FindWho);
	    while (dummylink) {
		dummylist = *dummylink;
		if (((who *)dummylist->cell)->chanop) {
		    dummyhack = CreateHack(((who *)dummylist->cell)->chan,
					   ((who *)dummylist->cell)->nick,
					   ((who *)dummylist->cell)->addr);
		    AddCellFirst(&bot->hack_list, CELL_HACKS, dummyhack);
		    printf("adding split %s!%s on %s\n",dummyhack->nick,
			   dummyhack->addr, dummyhack->chan);
		}
		FreeLink(dummylink);
		dummylink = FindCellAddr(&bot->who_list, dummywho, FindWho);
	    }
	    FreeCell(CELL_WHO, dummywho);
	} else {
	    dummywho = CreateWho(nick, NULL, NULL);
	    dummylink = FindCellAddr(&bot->who_list, dummywho, FindWho);
	    while (dummylink) {
		FreeLink(dummylink);
		dummylink = FindCellAddr(&bot->who_list, dummywho, FindWho);
	    }
	    FreeCell(CELL_WHO, dummywho);
	}
	return;
    }
    
    if (!strcmp(function, "TOPIC")) {
	int same_topic = 1;
	 
	dummychan = CreateChannel(msg_to, 0, (char *)NULL, 0, 0);
	dummylist = FindCell(&bot->chan_list, dummychan, FindChannel);
	FreeCell(CELL_CHANS, dummychan);
	if (dummylist) {
	    dummychan = (chans *)dummylist->cell;
	}
	if (dummychan) {
	    param++;
	    param1 = param;
	    while (*param1 != '\r' && *param1 != '\n' && *param1)
		param1++;
	    if (*param1)
		*param1 = 0;
	     
	    if (!strcmp(nick,((nicks *)bot->nick_list->cell)->nick)) {
		if (dummychan->topic)
		    free(dummychan->topic);
		dummychan->topic = (char *)malloc(strlen(param)+1);
		strcpy(dummychan->topic, param);
	    } else {
		if (dummychan->topic) {
		    param1 = param + strlen(param) - 1;
		    while (*param1 == ' ' && param1 >= param)
			param1--;
		    param2 = dummychan->topic + strlen(dummychan->topic) - 1;
		    while (*param2 == ' ' && param2 >= dummychan->topic)
			param2--;
		    while (param1 >= param && param2 >= dummychan->topic)
			if (*param1-- != *param2--) {
			    same_topic = 0;
			    break;
			}
		} else
		    same_topic = 0;
           
		dummyuser = CreateUser(addr, msg_to, US_MASTER|US_FRIEND);
		dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
		FreeCell(CELL_USERS, dummyuser);
		if ((dummychan->flags & LOCK_TOPIC)
		   && !dummylist && !same_topic) {
		    if (strcmp(nick,((nicks *)bot->nick_list->cell)->nick)) {
			if (dummychan->topic) {
			    sprintf(send_socket_buffer, "TOPIC %s :%s\n",
				    msg_to,
				    dummychan->topic);
			} else {
			    sprintf(send_socket_buffer, "TOPIC %s :No topic is set.\n",
				    msg_to);
			}
			SendSocket(bot->socknum, send_socket_buffer);
		    }
		} else {
		    if (dummychan->topic)
			free(dummychan->topic);
		    dummychan->topic = (char *)malloc(strlen(param)+1);
		    strcpy(dummychan->topic, param);
		}
	    }
	}
	return;
    }
    
    if (!strcmp(function,"JOIN")) {
	char *cg;
	cg = strchr(msg_to, '');
	if (cg)
	    *cg = 0;
	if (!strcmp(nick,((nicks *)bot->nick_list->cell)->nick)) {
	    sprintf(send_socket_buffer,"WHO %s\nMODE %s\n", msg_to, msg_to);
	    SendSocket(bot->socknum, send_socket_buffer);
	    dummychan = CreateChannel(msg_to, 0, (char *)NULL,0 ,0);
	    if (cg)
		dummychan->iamchanop = 1;
	    dummylist = FindCell(&bot->chan_list, dummychan, FindChannel);
	    FreeCell(CELL_CHANS, dummychan);
	    if (!dummylist) {
		dummychan = CreateChannel(msg_to, KEEP_CHAN|REVENGE|NO_HACK, NULL, 0, PLUS_N|PLUS_T);
		AddCellFirst(&bot->chan_list, CELL_CHANS, dummychan);
	    }
	    return;
	} else {
	    dummywho = CreateWho(nick, addr, msg_to);
	    AddCellFirst(&bot->who_list, CELL_WHO, dummywho);
	    dummyuser = CreateUser(addr, msg_to, US_SHIT);
	    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
	    the_user = (users *)((dummylist) ? dummylist->cell : NULL);
	    FreeCell(CELL_USERS, dummyuser);
	    if (the_user) {
		sprintf(send_socket_buffer,"MODE %s -o+b %s *!%s\n", msg_to, nick, the_user->addr);
		SendSocket(bot->socknum, send_socket_buffer);
		Kick(bot, msg_to, nick, "*shitlist*");
		return;
	    }
	    if (strchr(addr,'?')||strchr(addr,'*')) {
		banned_addr = Ban(bot, msg_to, addr); 
		Kick(bot, msg_to, nick, "lame fake -> outta here!");
		sprintf(send_socket_buffer,"mOdE %s -b *!%s\n", msg_to, banned_addr);
		free(banned_addr);
		dummytodo = CreateTodo(send_socket_buffer, 0);
		dummylist = FindCell(&bot->todo_list, dummytodo, FindTodo);
		FreeCell(CELL_TODOS, dummytodo);
		if (dummylist)
		    ((todos *)dummylist->cell)->when = time((time_t *)NULL);
		else
		    AddCellFirst(&bot->todo_list, CELL_TODOS,
				 CreateTodo(send_socket_buffer, bot->lame_fake_time + (rand()%LAME_FAKE_TIME_SKEW)));
		return;
	    }
	    dummyuser = CreateUser(addr, msg_to, US_AUTOOP);
	    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
	    FreeCell(CELL_USERS, dummyuser);
	    if (dummylist) {
		dummyhack = CreateHack(msg_to, nick, addr);
		dummylist = FindCell(&bot->hack_list, dummyhack, FindHack);
		FreeCell(CELL_HACKS, dummyhack);
		if (dummylist) {
		    sprintf(send_socket_buffer,"mOdE %s +o %s\n", msg_to, nick);
		    
		    AddCellFirst(&bot->todo_list,CELL_TODOS,
				 CreateTodo(send_socket_buffer, bot->hack_reop_time + (rand()%HACK_REOP_TIME_SKEW)));
		} else
		    Mode(bot, msg_to, "+o", nick);
	    }
	    if (cg) {
		char raw_mode[64];
		hosts *curhost;
		curhost = (hosts *)bot->host_list->cell;
		sprintf(raw_mode,"+%c %s", *++cg, nick);
		printf("***ServerOP %s: %s %s -> %s\n", curhost->name, nick, msg_to, raw_mode);
		ModeAnalysis(bot, msg_to, raw_mode, curhost->name, curhost->name);
	    }
	}
	return;
    }

    if (!strcmp(function,"KICK")) {
	param1 = param;
	while ((*param1!=' ') && (*param1!='\n') && *param1)
	    param1++;
	if (*param1)
	    *param1++=0;

	if (!strcmp(param,((nicks *)bot->nick_list->cell)->nick)) {
	    dummychan = CreateChannel(msg_to, 0, (char *)NULL,0 ,0);
	    dummylist = FindCell(&bot->chan_list, dummychan, FindChannel);
	    FreeCell(CELL_CHANS, dummychan);
	    if (dummylist) {
		dummychan = (chans *)dummylist->cell;
		if (dummychan->passwd) {
		    sprintf(send_socket_buffer,"%s %s",msg_to, dummychan->passwd);
		    Join(bot, send_socket_buffer);
		}
		else
		    Join(bot, msg_to);
	    }
	    dummywho = CreateWho(NULL, NULL, msg_to);
	    dummylink = FindCellAddr(&bot->who_list, dummywho, FindWhoLeave);
	    while (dummylink) {
		FreeLink(dummylink);
		dummylink = FindCellAddr(&bot->who_list, dummywho, FindWhoLeave);
	    }
	    FreeCell(CELL_WHO, dummywho);
	    fprintf(bot->outfile,"**** KICKED BY %s!%s (%s) reason %s\n",nick, addr, msg_to, param1);
	    fflush(bot->outfile);
	    dummyuser = CreateUser(addr, msg_to, US_MASTER|US_FRIEND|US_USER|US_OPRO);
	    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
	    FreeCell(CELL_USERS, dummyuser);
	    if (!dummylist) {
		banned_addr = (char *)malloc(strlen(addr)+16);
		if (IsNumeric(addr))
		  StripNumeric(addr, banned_addr);
		else
		  Strip(addr, banned_addr);
		UpdateUser(&bot->us_list, addr, msg_to, US_REBAN, bot->revenge_time, FindUserStrict);
		free(banned_addr);
	    }
	} else {
	    dummywho = CreateWho(param, NULL, msg_to);
	    dummylink = FindCellAddr(&bot->who_list, dummywho, FindWhoChan);
	    FreeLink(dummylink);
	    FreeCell(CELL_WHO, dummywho);
	}
	return;
    }

    if (!strcmp(function,"PART")) {
	if (!strcmp(nick,((nicks *)bot->nick_list->cell)->nick)) {
	    dummywho = CreateWho(NULL, NULL, msg_to);
	    dummylink = FindCellAddr(&bot->who_list, dummywho, FindWhoLeave);
	    while (dummylink) {
		FreeLink(dummylink);
		dummylink = FindCellAddr(&bot->who_list, dummywho, FindWhoLeave);
	    }
	    FreeCell(CELL_WHO, dummywho);
	    dummychan = CreateChannel(msg_to, 0, (char *)NULL,0 ,0);
	    dummylink = FindCellAddr(&bot->chan_list, dummychan, FindChannel);
	    if (dummylink)
		FreeLink(dummylink);
	    FreeCell(CELL_CHANS, dummychan);
	} else {
	    dummywho = CreateWho(nick, NULL, msg_to);
	    dummylink = FindCellAddr(&bot->who_list, dummywho, FindWhoChan);
	    FreeLink(dummylink);
	    FreeCell(CELL_WHO, dummywho);
	}
	return;
    }

    if (!strcmp(function,"NICK")) {
	param = msg_to + (*msg_to==':');
	if (strcasecmp(param, nick)) {
	    dummywho = CreateWho(nick, NULL, NULL);
	    dummylist = FindCell(&bot->who_list, dummywho, FindWho);
	    while (dummylist) {
		free(((who *)dummylist->cell)->nick);
		((who *)dummylist->cell)->nick = malloc(strlen(param)+1);
		strcpy(((who *)dummylist->cell)->nick, param);
		dummylist = FindCell(&dummylist, dummywho, FindWho);
	    }
	    FreeCell(CELL_WHO, dummywho);
	}
    }
    
    if (!strcmp(function,"INVITE")) {
	if (*param==':')
	    param++;
	dummyuser = CreateUser(addr, param, US_USER|US_FRIEND|US_MASTER);
	dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
	FreeCell(CELL_USERS, dummyuser);
	if (dummylist) {
	    Join(bot, param);
	    fprintf(bot->outfile,"****INVITED by %s!%s on channel %s\n",nick, addr, param);
	    fflush(bot->outfile);
	    dummychan = CreateChannel(param, 0, (char *)NULL,0 ,0);
	    dummylist = FindCell(&bot->chan_list, dummychan, FindChannel);
	    FreeCell(CELL_CHANS, dummychan);
	    if (!dummylist) {
		dummychan = CreateChannel(param, KEEP_CHAN|REVENGE|NO_HACK, NULL, 0, PLUS_N|PLUS_T);
		AddCellFirst(&bot->chan_list, CELL_CHANS, dummychan);
	    }
	}
	return;
    }
    
    if (!strcmp(function,"433") || /* nickname already in use */
	(!strcmp(function, "437") && bot->last_action == ACTION_NICK)) {
	/* Nick temporarily unavailable (and its nasty kludge) */
 	ChangeNick(bot);
	return;
    }

    if (!strcmp(function,"436")) { /* nickname collision */
	SendSocket(bot->socknum, "QUIT :Avoiding Nick Kill\n");
	close(bot->socknum);
	bot->state = STATE_NOT_CONNECTED;
	if (bot->behaviour & B_LOOP_SERV_ON_COLL)
	    LoopList(&bot->host_list);
	if (bot->behaviour & B_LOOP_NICK_ON_COLL)
	    LoopList(&bot->nick_list);  /* change nick to avoid more kills */
	FreeList(&bot->who_list);
	ShortTime(send_socket_buffer);
	fprintf(bot->outfile, "*** NICK COLLISION [%s] from %s\n", send_socket_buffer, param);
	fflush(bot->outfile);
	return;
    }

    if (!strcmp(function,"KILL")) { /* KILL */
	close(bot->socknum);
	bot->state = STATE_NOT_CONNECTED;
	if (bot->behaviour & B_LOOP_SERV_ON_KILL)
	    LoopList(&bot->host_list);
	if (bot->behaviour & B_LOOP_NICK_ON_KILL)
	    LoopList(&bot->nick_list);  /* change nick to avoid more kills */
	FreeList(&bot->who_list);
	ShortTime(send_socket_buffer);
	fprintf(bot->outfile, "*** KILL [%s] from %s!%s\n", send_socket_buffer, nick, addr);
	fflush(bot->outfile);
	return;
    }
    
    if (!strcmp(function,"352")) { /* who reply */
	param1 = param;
	while ((*param1!=' ') && *param1)
	    param1++;
	if (*param1)
	    *param1++=0;
	
	param2 = param1;
	while ((*param2!=' ') && *param2)
	    param2++;
	if (*param2)
	    *param2++='@';
    
	while ((*param2!=' ') && *param2)
	    param2++;
	if (*param2)
	    *param2++=0;
	while ((*param2!=' ') && *param2)
	    param2++;
	if (*param2)
	    *param2++=0;

	param3 = param2;
	while ((*param3!=' ') && *param3)
	    param3++;
	if (*param3)
	    *param3++=0;

	param4 = param3;
	while ((*param4!=' ') && *param4)
	    param4++;
	if (*param4)
	    *param4++=0;
	
	dummywho = CreateWho(param2, param1, param);
	dummywho->chanop = (strchr(param3,'@')!=NULL);
	AddCellFirst(&bot->who_list, CELL_WHO, dummywho);
	if (!strcmp(param2,((nicks *)bot->nick_list->cell)->nick)) {
	    dummychan = CreateChannel(param, 0, (char *)NULL,0 ,0);
	    dummylist = FindCell(&bot->chan_list, dummychan, FindChannel);
	    FreeCell(CELL_CHANS, dummychan);
	    if (dummylist) {
		dummychan = (chans *)dummylist->cell;
		dummychan->iamchanop = (strchr(param3,'@')!=NULL);
	    }
	}
	return;
    }
    
    if (!strcmp(function,"001")) {
	sprintf(send_socket_buffer,"MODE %s +i\n", ((nicks *)bot->nick_list->cell)->nick);
	SendSocket(bot->socknum, send_socket_buffer);
	dummylist = bot->chan_list;
	while (dummylist) {
	    if (((chans *)dummylist->cell)->passwd) {
		sprintf(send_socket_buffer,"%s %s",((chans *)dummylist->cell)->name, ((chans *)dummylist->cell)->passwd);
		Join(bot, send_socket_buffer);
	    } else
		Join(bot, ((chans *)dummylist->cell)->name);
	    dummylist = dummylist->next;
	}
	((hosts *)bot->host_list->cell)->uptime = time((time_t *)NULL);
	return;
    }

    /* a fake event */
    if (!strcmp(function,"999")) {
	dummylist = bot->chan_list;
	while (dummylist) {
	    if (((chans *)dummylist->cell)->passwd) {
		sprintf(send_socket_buffer,"%s %s",((chans *)dummylist->cell)->name, ((chans *)dummylist->cell)->passwd);
		Join(bot, send_socket_buffer);
	    } else
		Join(bot, ((chans *)dummylist->cell)->name);
	    dummylist = dummylist->next;
	}
	return;
    }

    if (!strcmp(function,"302")) { /* userhost reply */
	param1 = param + (*param == ':');
	if (*param1)
	    ClientParseUserhost(bot, param1);
	else
	    FreeLink(&bot->uh_request);
	return;
    }

    if (!strcmp(function,"367")) { /* banlist reply */
	param1 = param + (*param == ':'); /* param1 = channel, param2 = pattern */
	param2 = param1;
	while ((*param2!=' ') && *param2)
	    param2++;
	if (*param2)
	    *param2++=0;
	while ((*param2!='!') && *param2)
	    param2++;
	if (*param2)
	    *param2++=0;

	AddCellFirst(&bot->ban_list, CELL_BAN, CreateBan(param1, param2));
	return;
    }
    
    if (!strcmp(function,"368")) { /* end of banlist */
	param1 = param;       /* param = channel */
	while ((*param1!=' ') && *param1)
	    param1++;
	if (*param1)
	    *param1++=0;

	ClientParseBan(bot, param);
	return;
    }

    if (!strcmp(function, "471") || /* channel full */
	!strcmp(function, "473") || /* invite only channel */
	!strcmp(function, "474") || /* banned from channel */
	!strcmp(function, "475") || /* bad channel key */
	(!strcmp(function, "437") && bot->last_action == ACTION_CHAN)) {
                             /* Nick/channel is temporarily unavailable */
	param1 = param;       /* param = channel */
	while ((*param1!=' ') && *param1)
	    param1++;
	if (*param1)
	    *param1++=0;
	
	if (!strcmp(function, "437")) {
	    fprintf(bot->outfile, "*** CHANNEL DELAY on %s\n", param);
	    fflush(bot->outfile);
	    /* exit asap, as the retries will help on this one */
	    return;
	}

	dummychan = CreateChannel(param, 0, (char *)NULL,0 ,0);
	dummylist = FindCell(&bot->chan_list, dummychan, FindChannel);
	FreeCell(CELL_CHANS, dummychan);
	if (dummylist) {
	    if (((chans *)dummylist->cell)->passwd)
		sprintf(send_socket_buffer,"JOIN %s %s", param,
			((chans *)dummylist->cell)->passwd);
	    else
		sprintf(send_socket_buffer,"JOIN %s\n", param);
	} else
	    sprintf(send_socket_buffer,"JOIN %s\n", param);
	
	AddCellFirst(&bot->todo_list, CELL_TODOS, 
		     CreateTodo(send_socket_buffer, bot->rejoin_time));
	return;
    }

    if (!strcmp(function, "451")) { /* Not registered */
	SendSocket(bot->socknum,"QUIT :riding the net\n"); 
	shutdown(bot->socknum,2); /* switch to another server */
	close(bot->socknum);
	bot->state = STATE_NOT_CONNECTED;
	FreeList(&bot->who_list);
	LoopList(&bot->host_list);
    }

    if (!strcmp(function, "332")) { /* TOPIC changed */
	char todo[512];
	int same_topic = 1;
	
	param1 = param;
	while (*param1 != ' ' && *param1)
	    param1++;
	if (*param1)
	    *param1++ = 0;
	if (*param1 == ':')
	    param1++;
	param2 = param1;
	while (*param2 != '\r' && *param2 != '\n' && *param2)
	    param2++;
	if (*param2)
	    *param2 = 0;
	dummychan = CreateChannel(param, 0, (char *)NULL, 0, 0);
	dummylist = FindCell(&bot->chan_list, dummychan, FindChannel);
        FreeCell(CELL_CHANS, dummychan);
	if (dummylist)
	    dummychan = (chans *)dummylist->cell;
	if (dummychan) {
	    if (dummychan->flags & LOCK_TOPIC) {
		if (dummychan->topic) {
		    param2 = param1 + strlen(param1) - 1;
		    while (*param2 == ' ' && param2 >= param1)
			param2--;
		    param3 = dummychan->topic + strlen(dummychan->topic) - 1;
		    while (*param3 == ' ' && param3 >= dummychan->topic)
			param3--;
		    while (param2 >= param1 && param3 >= dummychan->topic)
			if (*param2-- != *param3--) {
			    same_topic = 0;
			    break;
			}
		} else
		    same_topic = 0;
		if (same_topic) {
		    if (dummychan->topic)
			free(dummychan->topic);
		    dummychan->topic = (char *)malloc(strlen(param1)+1);
		    strcpy(dummychan->topic, param1);
		} else {
		    if (dummychan->topic)
			sprintf(todo, "TOpic %s :%s\n",
				param, dummychan->topic);
		    else
			sprintf(todo, "TOpic %s :No topic is set.\n", param);
		    if (dummychan->iamchanop)
			SendSocket(bot->socknum, todo);
		    else
			AddCellFirst(&bot->todo_list, CELL_TODOS,
				     CreateTodo(todo, TOPIC_ON_JOIN_DELAY));
		}
	    } else {
		if (dummychan->topic)
		    free(dummychan->topic);
		dummychan->topic = (char *)malloc(strlen(param1)+1);
		strcpy(dummychan->topic, param1);
	    }
	}
	return;
    }
}

void ServerParse(bot, sock_read) 
    irc_bot *bot;
    char *sock_read;
{
    /* do nothing */
}

void ServiceParse(bot, sock_read)
    irc_bot *bot;
    char *sock_read;
{
    /* do nothing */
}


void Parse(bot, sock_read)
    irc_bot *bot;
    char *sock_read;
{
    if (bot->behaviour & B_CLIENT) {
	ClientParse(bot, sock_read);
    } else if (bot->behaviour & B_SERVER) {
	ServerParse(bot, sock_read);
    } else if (bot->behaviour & B_SERVICE) {
	ServiceParse(bot, sock_read);
    } else {
	/* ahem, a "lost in space" bot :) */
    }
}
