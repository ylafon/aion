/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: client.c,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

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

/*
 *
 *     Parse public commands regarding the level of the user
 *
 */

void ClientParsePublic(bot, nick, addr, channel, param)
    irc_bot *bot;
    char *nick, *addr, *channel, *param;
{
    char  *param1,*param2,*param3,*param4,*param5,*pos;
    char  send_socket_buffer[512];
    char  *send_socket_pos,*banned_addr,*user_addr;
    char  command[512];
    char  ban_string[128];
    char  op_string[128];
    users *the_user, *dummyuser;
    chans *the_chan;
    todos *dummytodo;
    list  *dummylist, *linklist;
    who   *dummywho;
    int req_time, req_mode;
    int flags, num, i;
    int nbmods, umodes;
    int nbbads, ubads;
    int act;

    nbmods = 0;
    nbbads = 0;
    act    = 0;
    param3 = NULL;

    param1 = param+(*param==':');
    while ((*param1==' ') && *param1)
	param1++;

/*
 *   if the first parameter begins with ctrl-a (001) then it must be a CTCP request 
 */

    if (*param1 == '') {
	param2 = param1;
	while ((*param2!=' ') && *param2)
	    param2++;
	if (*param2)
	    *param2++=0;
	HandleCtcp(bot,nick,addr,param1,param2);
	return;
    }

    /**
     * now get the channel and do the preprocessing first
     */
    
    
    the_chan = CreateChannel(channel, 0, (char *)NULL, 0, 0);
    dummylist = FindCell(&bot->chan_list, the_chan, FindChannel);
    FreeCell(CELL_CHANS, the_chan);
    if (!dummylist)
	return;
    the_chan = (chans *)dummylist->cell;
    if (the_chan->flags & GATEWAY_LOCAL_CHAN) {
	if (*channel == '#') {
	    sprintf(send_socket_buffer, "<%s> %s", nick, param1);
	    *channel = '&';
	    Notice(bot, channel, "internal", send_socket_buffer);
	    *channel = '#';
	}
    }

/*
 *  Rot13 command type... a rot13 command begins with ~
 */ 
    
    if (*param1 == ROT13_CHAR) {
	param1++;
	Rot13(param1);
	ClientParsePublic(bot, nick, addr, channel, param1);
	return;
    }

/*
 *  Encode command type... an encode command s=begins with #
 */

    if (*param1 == ENCODE_CHAR) {
	param1++;
	Decode(param1);
	ClientParsePublic(bot, nick, addr, channel, param1);
	return;
    }

/*
 *     now we must get the internal channel from the name
 */

    /* test if we must log */
    if (the_chan->flags & LOG_CHAN) {
	/* reopen the log if not present */
	if (the_chan->flags & LOGTIME_GMT)
	    GmtTime(send_socket_buffer);
	else
	    ShortTime(send_socket_buffer);
	if (!the_chan->log) {
	    the_chan->log = fopen(the_chan->name, "a");
	    fprintf(the_chan->log, "*** [%s] STARTING LOG\n",
		    send_socket_buffer);
	}
	if (*param) {
	    fprintf(the_chan->log, "[%s] <%s!%s> %s\n", 
		    send_socket_buffer, nick, addr, param+1);
	    fflush(the_chan->log);
	}
    }
    
/*
 *   Test if first character is the command character otherwise, exit fast
 */
    
    if (*param1++ == bot->cmd_char) {
	ShortTime(send_socket_buffer);
	sprintf(command,"%s| ACTION from %s!%s on %s: %s\n", send_socket_buffer, nick, addr, channel, param1);
	
	send_socket_pos = send_socket_buffer;
	
/*
 *     now we must get the user level from its address
 */

	dummyuser = CreateUser(addr, channel, US_MASTER|US_FRIEND|US_USER);
	dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
	the_user = (users *)((dummylist) ? dummylist->cell : NULL);

	param2 = param1;
	while ((*param2!=' ') && *param2)
	    param2++;
	if (*param2)
	    *param2++=0;

#ifdef DEBUG
	printf("command [%s]\n",param1);
#endif /* DEBUG */

	flags = (the_user) ? (the_user->flags & (US_MASTER|US_FRIEND|US_USER)) : 0;
	
	while (dummylist) {
	    flags |= (((users *)dummylist->cell)->flags & (US_MASTER|US_FRIEND|US_USER));
	    dummylist = dummylist->next;
	    if (dummylist)
		dummylist = FindCell(&dummylist, dummyuser, FindUser);
	}
	FreeCell(CELL_USERS, dummyuser);
	switch(flags) {
/*
 *     First of all, the Master commands...
 *       keep [lkipsnmtob], noop, allowop, norevenge, allowrevenge, nohack,
 *       allowhack, die [reason], saveall, rehash (bugged), adduser [fubakop] [nick] [time] [channel],
 *       addshit [nick] [time] [channel], deluser (not implemented), loopserv, userinfo [nick] [channel]
 */
	    /**
	     * status has a special status as you can rewrite the param1
	     * If another may be reused that way... 
	     */
	case US_MASTER:
	case (US_MASTER|US_FRIEND):
	case (US_MASTER|US_USER):
	case (US_MASTER|US_FRIEND|US_USER):
	    if (!strcmp(param1,"keep")) {
		pos = param2;
		flags = the_chan->flags & (REVENGE|NO_HACK);
		while (*pos) {
		    switch(*pos) {
		    case 'l':
			flags |= KEEP_L;
			break;
		    case 'k':
			flags |= KEEP_K;
			break;
		    case 'i':
			flags |= KEEP_I;
			break;
		    case 'p':
			flags |= KEEP_P;
			break;
		    case 's':
			flags |= KEEP_S;
			break;
		    case 'n':
			flags |= KEEP_N;
			break;
		    case 'm':
			flags |= KEEP_M;
			break;
		    case 't':
			flags |= KEEP_T;
			break;
		    case 'o':
			flags |= KEEP_O;
			break;
		    case 'b':
			flags |= KEEP_B;
			break;
		    }
		    pos++;
		}
		the_chan->flags = flags;
		strcpy(param1,"status");
	    }

	    if (!strcmp(param1,"noop")) {
		the_chan->flags |= KEEP_O;
		strcpy(param1,"status");
	    } else if (!strcmp(param1,"allowop")) {
		the_chan->flags &= ~KEEP_O;
		strcpy(param1,"status");
	    } else if (!strcmp(param1,"norevenge")) {
		the_chan->flags &= ~REVENGE;
		strcpy(param1,"status");
	    } else if (!strcmp(param1,"allowrevenge")) {
		the_chan->flags |= REVENGE;
		strcpy(param1,"status");
	    } else if (!strcmp(param1,"nomode")) {
		the_chan->flags &= ~FORCE_MODE;
		strcpy(param1,"status");
	    } else if (!strcmp(param1,"allowmode")) {
		the_chan->flags |= FORCE_MODE;
		strcpy(param1,"status");
	    } else if (!strcmp(param1,"nohack")) {
		the_chan->flags |= NO_HACK;
		strcpy(param1,"status");
	    } else if (!strcmp(param1,"allowhack")) {
		the_chan->flags &= ~NO_HACK;
		strcpy(param1,"status");
	    } else if (!strcmp(param1,"gateway")) {
		the_chan->flags |= GATEWAY_LOCAL_CHAN;
		strcpy(param1,"status");
	    } else if (!strcmp(param1,"stopgateway")) {
		the_chan->flags &= ~GATEWAY_LOCAL_CHAN;
		strcpy(param1,"status");
	    } else if (!strcmp(param1,"startlog")) {
		if (the_chan->flags & LOGTIME_GMT)
		    GmtTime(send_socket_buffer);
		else
		    ShortTime(send_socket_buffer);
		the_chan->flags |= LOG_CHAN;
		if (!the_chan->log)
		    the_chan->log = fopen(the_chan->name, "a");
		fprintf(the_chan->log, "*** [%s] STARTING LOG\n",
			send_socket_buffer);
		strcpy(param1,"status");
	    } else if (!strcmp(param1,"stoplog")) {
		if (the_chan->flags & LOGTIME_GMT)
		    GmtTime(send_socket_buffer);
		else
		    ShortTime(send_socket_buffer);
		the_chan->flags &= ~LOG_CHAN;
		if (the_chan->log) {
		    fprintf(the_chan->log,"*** [%s] STOPPING LOG\n",
			    send_socket_buffer);
		    fclose(the_chan->log);
		    the_chan->log = NULL;
		}
		strcpy(param1,"status");
	    } else if (!strcmp(param1,"gmt")) {
		the_chan->flags |= LOGTIME_GMT;
		strcpy(param1,"status");
	    } else if (!strcmp(param1,"local"))
	    {
		the_chan->flags &= ~LOGTIME_GMT;
		strcpy(param1,"status");
	    } else if (!strcmp(param1,"die")) {
		act++;
		if (*param2) {
		    sprintf(send_socket_buffer,"QUIT :%s\n",param2);
		    SendSocket(bot->socknum, send_socket_buffer);
		} else
		    SendSocket(bot->socknum,"QUIT :BBL\n");
		bot->quit = TRUE;
		break;
	    } 
	    
	    if (!strcmp(param1,"saveall")) {
		act++;
		SaveAll(bot);
		break;
	    } 
	    
	    if (!strcmp(param1,"rehash")) {
		act++;
		LoadAll(bot);
		break;
	    } 
	    
	    if (!strcmp(param1,"adduser")) {
		/*
		 * param2: mode
		 * param3: nick then addr
		 * param4: time
		 * param5: channel
		 */
	
		act++;
		param3 = param2;
		while ((*param3!=' ') && (*param3!='\n') && *param3)
		    param3++;
		if (*param3)
		    *param3++=0;
		
		param4 = param3;
		while ((*param4!=' ') && (*param4!='\n') && *param4)
		    param4++;
		if (*param4)
		    *param4++=0;
		
		param5 = param4;
		while ((*param5!=' ') && (*param5!='\n') && *param5)
		    param5++;
		if (*param5)
		    *param5++=0;	

		if (!*param5)
		    param5 = channel;

		req_mode = 0;
		req_time = atoi(param4);
		if (req_time<0)
		    req_time = ETERNITY;

		for (pos=param2; *pos; pos++)
		    switch(*pos) {
		    case 'f':
			req_mode |= US_FRIEND;
			break;
		    case 'u':
			req_mode |= US_USER;
			break;
		    case 'b':
			req_mode |= US_BOT;
			break;
		    case 'a':
			req_mode |= US_AUTOOP;
			break;
		    case 'k':
			req_mode |= US_KPRO;
			break;
		    case 'o':
			req_mode |= US_OPRO;
			break;
		    case 'p':  /* b was used for bot */
			req_mode |= US_BPRO;
			break;
		    }
		dummywho = CreateWho(param3, NULL, param5);
		dummylist = FindCell(&bot->who_list, dummywho, FindWho);
		FreeCell(CELL_WHO,dummywho);
		if (dummylist) {
		    param3 = ((who *)dummylist->cell)->addr;
		    user_addr = (char *)malloc(strlen(param3)+16);
		    if (IsNumeric(param3))
			StripNumeric(param3, user_addr);
		    else
			Strip(param3, user_addr);
		    UpdateUser(&bot->us_list, user_addr, param5, req_mode, req_time, FindUserStrict);
		    free(user_addr);
		} else {
		    sprintf(send_socket_buffer,"User %s must be on the channel", param3);
		    Notice(bot, nick, addr, send_socket_buffer);
		}
		break; /* must do this  adduser nick/addr [time] [channel] */
	    } 
	    
	    if (!strcmp(param1,"addshit")) {
		/*
		 * param2: nick or addr
		 * param3: time
		 * param4: channel
		 */
	
		act++;
		param3 = param2;
		while ((*param3!=' ') && (*param3!='\n') && *param3)
		    param3++;
		if (*param3)
		    *param3++=0;
		
		param4 = param3;
		while ((*param4!=' ') && (*param4!='\n') && *param4)
		    param4++;
		if (*param4)
		    *param4++=0;
		
		if (!*param4)
		    param4 = channel;

		req_mode = 0;
		req_time = atoi(param3);
		if (req_time<0)
		    req_time = ETERNITY;

		if (strchr(param2,'*')||strchr(param2,'@')||strchr(param2,'.')) {
		    if (strchr(param2,'@'))
			dummyuser = CreateUser(param2, param4, US_BPRO);
		    else {
			sprintf(ban_string, "*@%s", param2);
			dummyuser = CreateUser(ban_string, param4, US_BPRO);
		    }
		    dummylist = FindCell(&bot->us_list, dummyuser, FindBanUser);
		    FreeCell(CELL_USERS, dummyuser);
		    if (!dummylist)
			UpdateUser(&bot->us_list, ban_string, param4, US_REBAN|US_SHIT, req_time, FindUserStrict);
		    else {
			sprintf(send_socket_buffer,"Can't shit User %s on this channel", param2);
			Notice(bot, nick, addr, send_socket_buffer);
		    }
		} else {
		    dummywho = CreateWho(param2, NULL, param4);
		    dummylist = FindCell(&bot->who_list, dummywho, FindWho);
		    FreeCell(CELL_WHO,dummywho);
		    if (dummylist) {
			param2 = ((who *)dummylist->cell)->addr;
			user_addr = (char *)malloc(strlen(param2)+16);
			if (IsNumeric(param2))
			    StripNumeric(param2, user_addr);
			else
			    Strip(param2, user_addr);
			UpdateUser(&bot->us_list, user_addr, param4, US_REBAN|US_SHIT, req_time, FindUserStrict);
			free(user_addr);
		    } else {
			sprintf(send_socket_buffer,"User %s must be on the channel", param3);
			Notice(bot, nick, addr, send_socket_buffer);
		    }
		}
		break; 
	    } 
	    
	    if (!strcmp(param1,"deluser")) {
		act++;
		break; /* FIXME must do this  deluser nick/addr [channel] */
	    } 
	    
	    if (!strcmp(param1,"loopserv")) {
		act++;
		SendSocket(bot->socknum,"QUIT :riding the net\n");
		shutdown(bot->socknum,2);
		close(bot->socknum);
		bot->state = STATE_NOT_CONNECTED;
		FreeList(&bot->who_list);
		LoopList(&bot->host_list);
		break;
	    } 
	    
	    if (!strcmp(param1,"userinfo")) {
		act++;
		param3 = param2;
		while ((*param3!=' ') && *param3)
		    param3++;
		if (*param3)
		    *param3++=0;
		
		if (!*param3)
		    param3 = channel;

		dummywho = CreateWho(param2, NULL, param3);
		dummylist = FindCell(&bot->who_list, dummywho, FindWho);
		FreeCell(CELL_WHO,dummywho);
		if (dummylist) {
		    flags = 0;
		    umodes = 0;
		    ubads = 0;
		    dummyuser = CreateUser(((who *)dummylist->cell)->addr, param3,
					   US_MASTER|US_FRIEND|US_USER|US_OPRO|US_KPRO|US_BPRO|US_AUTOOP);
		    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
		    while (dummylist) {
			flags |= ((users *)dummylist->cell)->flags;
			umodes += ((users *)dummylist->cell)->actions;
			ubads += ((users *)dummylist->cell)->bad_actions;
			dummylist = dummylist->next;
			if (dummylist)
			    dummylist = FindCell(&dummylist, dummyuser, FindUser);
		    }
		    FreeCell(CELL_USERS, dummyuser);
		    if (flags) {
			sprintf(send_socket_pos,"%s is ", param2);
			send_socket_pos+=strlen(send_socket_pos);
			if (flags & US_MASTER)
			    *send_socket_pos++='m';
			if (flags & US_FRIEND)
			    *send_socket_pos++='f';
			if (flags & US_USER)
			    *send_socket_pos++='u';
			if (flags & US_BOT)
			    *send_socket_pos++='b';
			if (flags & US_AUTOOP)
			    *send_socket_pos++='a';
			if (flags & US_OPRO)
			    *send_socket_pos++='o';
			if (flags & US_KPRO)
			    *send_socket_pos++='k';
			if (flags & US_BPRO)
			    *send_socket_pos++='p';
			sprintf(send_socket_pos," action/bad %d/%d on %s", umodes, ubads, param3);
			Notice(bot, nick, addr, send_socket_buffer);
		    } else {
			sprintf(send_socket_pos,"%s is nothing on %s", param2, param3);
			Notice(bot, nick, addr, send_socket_buffer);
		    }
		} else {
		    AddCellLast(&bot->uh_request, CELL_UH, CreateUh(C_USERINFO, nick, param3, 0));
		    sprintf(send_socket_buffer, "USERHOST %s\n",param2);
		    SendSocket(bot->socknum, send_socket_buffer);
		}
		break;
	    } else if (!strcmp(param1, "command")) {
		act++;
		param3 = param2;
		while (*param3 == ' ' && *param3 != '\n' && *param3)
		    param3++;
		if (*param3 > 32 && *param3 < 127
		   && *param3 != ENCODE_CHAR && *param3 != ROT13_CHAR) {
		    bot->cmd_char = *param3;
		    if (bot->config_flag & FLAG_ADD_COMMAND_CHAR)
			bot->realname[strlen(bot->realname)-2] = *param3;
		}
		break;
	    }
/*
 *    Friend commands, including mass op/dop/k/kb
 */
	case US_FRIEND:
	case (US_FRIEND|US_USER):
	    if (!strcmp(param1, "leave")) {
		act++;
		sprintf(send_socket_buffer, "PART %s\n", channel);
		SendSocket(bot->socknum, send_socket_buffer);
		if (the_chan->flags & LOGTIME_GMT)
		    GmtTime(send_socket_buffer);
		else
		    ShortTime(send_socket_buffer);
		if (the_chan->log) {
		    fprintf(the_chan->log, "*** [%s] LEAVING CHAN\n",
			    send_socket_buffer);
		    fclose(the_chan->log);
		    the_chan->log = NULL;
		}
		break;
	    }
	    if (!strcmp(param1, "lock")) {
		act++;
		the_chan->flags |= LOCK_TOPIC;
		strcpy(param1, "status");
	    } else if (!strcmp(param1, "unlock")) {
		act++;
		the_chan->flags &= ~LOCK_TOPIC;
		strcpy(param1, "status");
	    }
	    
	    if (!strcmp(param1,"status")) {
		act++;
		strcpy(send_socket_buffer,"keeping ");
		send_socket_pos = send_socket_buffer + 8;
		flags = the_chan->flags;
		if (flags & KEEP_L)
		    *send_socket_pos++ = 'l';
		if (flags & KEEP_K)
		    *send_socket_pos++ = 'k';
		if (flags & KEEP_I)
		    *send_socket_pos++ = 'i';
		if (flags & KEEP_P)
		    *send_socket_pos++ = 'p';
		if (flags & KEEP_S)
		    *send_socket_pos++ = 's';
		if (flags & KEEP_M)
		    *send_socket_pos++ = 'm';
		if (flags & KEEP_N)
		    *send_socket_pos++ = 'n';
		if (flags & KEEP_T)
		    *send_socket_pos++ = 't';
		if (flags & KEEP_O)
		    *send_socket_pos++ = 'o';
		if (flags & KEEP_B)
		    *send_socket_pos++ = 'b';
		if (flags & NO_HACK) {
		    strcpy(send_socket_pos," NoHack");
		    send_socket_pos += strlen(send_socket_pos);
		}
		if (flags & REVENGE) {
		    strcpy(send_socket_pos," Revenge");
		    send_socket_pos += strlen(send_socket_pos);
		}
		if (flags & LOCK_TOPIC) {
		    strcpy(send_socket_pos, " LockedTopic");
		    send_socket_pos += strlen(send_socket_pos);
		}
		if (flags & FORCE_MODE) {
		    strcpy(send_socket_pos, " ForceMode");
		    send_socket_pos += strlen(send_socket_pos);
		}
		if (flags & LOG_CHAN) {
		    strcpy(send_socket_pos, " Logged");
		    send_socket_pos += strlen(send_socket_pos);
		    if (flags & LOGTIME_GMT) {
			strcpy(send_socket_pos, " GMT");
			send_socket_pos += strlen(send_socket_pos);
		    }
		}
		if (flags & GATEWAY_LOCAL_CHAN) {
		    strcpy(send_socket_pos, " Gateway");
		    send_socket_pos += strlen(send_socket_pos);
		}
		sprintf(send_socket_pos," on channel %s", channel);
		Notice(bot, nick, addr, send_socket_buffer);
		break;
	    }
	    
	    if (!strcmp(param1,"botstatus")) {
		act++;

		for (linklist = bot->chan_list; linklist; linklist = linklist->next) {		
		    strcpy(send_socket_buffer,"keeping ");
		    send_socket_pos = send_socket_buffer + 8;
		    flags = ((chans *)linklist->cell)->flags;

		    if (flags & KEEP_L)
			*send_socket_pos++ = 'l';
		    if (flags & KEEP_K)
			*send_socket_pos++ = 'k';
		    if (flags & KEEP_I)
			*send_socket_pos++ = 'i';
		    if (flags & KEEP_P)
			*send_socket_pos++ = 'p';
		    if (flags & KEEP_S)
			*send_socket_pos++ = 's';
		    if (flags & KEEP_M)
			*send_socket_pos++ = 'm';
		    if (flags & KEEP_N)
			*send_socket_pos++ = 'n';
		    if (flags & KEEP_T)
			*send_socket_pos++ = 't';
		    if (flags & KEEP_O)
			*send_socket_pos++ = 'o';
		    if (flags & KEEP_B)
			*send_socket_pos++ = 'b';
		    if (flags & NO_HACK) {
			strcpy(send_socket_pos," NoHack");
			send_socket_pos += strlen(send_socket_pos);
		    }
		    if (flags & REVENGE) {
			strcpy(send_socket_pos," Revenge");
			send_socket_pos += strlen(send_socket_pos);
		    }
		    if (flags & LOCK_TOPIC) {
			strcpy(send_socket_pos, " LockedTopic");
			send_socket_pos += strlen(send_socket_pos);
		    }
		    if (flags & FORCE_MODE) {
			strcpy(send_socket_pos, " ForceMode");
			send_socket_pos += strlen(send_socket_pos);
		    }
		    if (flags & LOG_CHAN) {
			strcpy(send_socket_pos, " Logged");
			send_socket_pos += strlen(send_socket_pos);
			if (flags & LOGTIME_GMT) {
			    strcpy(send_socket_pos, " GMT");
			    send_socket_pos += strlen(send_socket_pos);
			}
		    }
		    if (flags & GATEWAY_LOCAL_CHAN) {
			strcpy(send_socket_pos, " Gateway");
			send_socket_pos += strlen(send_socket_pos);
		    }
		    if (((chans *)linklist->cell)->iamchanop) {
			strcpy(send_socket_pos," OP");
			send_socket_pos += strlen(send_socket_pos);
		    } else {
			strcpy(send_socket_pos," OPLESS");
			send_socket_pos += strlen(send_socket_pos);
		    }	
		    flags = ((chans *)linklist->cell)->mode;
		    if (flags)
			*send_socket_pos++ = ' ';
		    if (flags & PLUS_I)
			*send_socket_pos++ = 'i';
		    if (flags & PLUS_P)
			*send_socket_pos++ = 'p';
		    if (flags & PLUS_S)
			*send_socket_pos++ = 's';
		    if (flags & PLUS_N)
			*send_socket_pos++ = 'n';
		    if (flags & PLUS_M)
			*send_socket_pos++ = 'm';
		    if (flags & PLUS_T)
			*send_socket_pos++ = 't';
		    if (flags & PLUS_K) {
			*send_socket_pos++ = 'k';
			sprintf(send_socket_pos," %s", ((chans *)linklist->cell)->passwd);
			send_socket_pos += strlen(send_socket_pos);
		    }
		    if (flags & PLUS_L) {
			*send_socket_pos++ = 'l';
			sprintf(send_socket_pos," %d", ((chans *)linklist->cell)->size);
			send_socket_pos += strlen(send_socket_pos);
		    }
		    sprintf(send_socket_pos," on channel %s", ((chans *)linklist->cell)->name);
		    send_socket_pos += strlen(send_socket_pos);
		    
		    Notice(bot, nick, addr, send_socket_buffer);
		}
		break;
	    }

	    if (!strcmp(param1,"op")) {
		act++;
		if (!(the_chan->flags & KEEP_O)) {
		    if (strchr(param2,'*')) {
			if (strchr(param2,'@'))
			    strcpy(op_string, param2);
			else
			    sprintf(op_string,"*@%s", param2);
			nbmods = 0;
			for (linklist = bot->who_list; linklist; linklist = linklist->next) {
			    if (!((who *)linklist->cell)->chanop
			       && Match(((who *)linklist->cell)->chan, channel)
			       && MatchAddr(((who *)linklist->cell)->addr, op_string)) {
				dummyuser = CreateUser(((who *)linklist->cell)->addr, channel, US_SHIT|US_REBAN);
				dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
				FreeCell(CELL_USERS, dummyuser);
				if (!dummylist) {
				    sprintf(send_socket_pos," %s", ((who *)linklist->cell)->nick);
				    send_socket_pos+=strlen(send_socket_pos);
				    nbmods++;
				    if (nbmods==MAX_MODES) {
					the_user->actions += MAX_MODES;
					nbmods = 0;
					Mode(bot, channel, POPSTR, send_socket_buffer+1);
					send_socket_pos = send_socket_buffer;
				    }
				}
			    }
			}
			if (nbmods)
			    Mode(bot, channel, POPSTR, send_socket_buffer+1);
			break;
		    }
		}
	    }
	    

	    if (!strcmp(param1,"dop")) {
		act++;
		if (!(the_chan->flags & KEEP_O)) {
		    if (strchr(param2,'*')) {
			if (strchr(param2,'@'))
			    strcpy(op_string, param2);
			else
			    sprintf(op_string,"*@%s", param2);
			nbmods = 0;
			for (linklist = bot->who_list; linklist; linklist = linklist->next) {
			    if (((who *)linklist->cell)->chanop
			       && Match(((who *)linklist->cell)->chan, channel)
			       && MatchAddr(((who *)linklist->cell)->addr, op_string)) {
				dummyuser = CreateUser(((who *)linklist->cell)->addr, channel, US_OPRO);
				dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
				FreeCell(CELL_USERS, dummyuser);
				if (!dummylist && 
				   strcmp(((who *)linklist->cell)->nick,((nicks *)bot->nick_list->cell)->nick)) {
				    sprintf(send_socket_pos," %s", ((who *)linklist->cell)->nick);
				    send_socket_pos+=strlen(send_socket_pos);
				    nbmods++;
				    if (nbmods==MAX_MODES) {
					the_user->actions += MAX_MODES;
					nbmods = 0;
					Mode(bot, channel, MOPSTR, send_socket_buffer+1);
					send_socket_pos = send_socket_buffer;
				    }
				}
			    }
			}
			if (nbmods)
			    Mode(bot, channel, MOPSTR, send_socket_buffer+1);
			break;
		    }
		}
	    }
	    
	    if (!strcmp(param1,"kb")) {
		act++;
		param3 = param2;
		while ((*param3!=' ') && *param3)
		    param3++;
		if (*param3)
		    *param3++=0;
		
		if (strchr(param2,'*')||strchr(param2,'@')||strchr(param2,'.')) {
		    if (strchr(param2,'@'))
			dummyuser = CreateUser(param2, channel, US_BPRO);
		    else {
			sprintf(ban_string, "*@%s", param2);
			dummyuser = CreateUser(ban_string, channel, US_BPRO);
		    }
		    dummylist = FindCell(&bot->us_list, dummyuser, FindBanUser);
		    if (!dummylist) {
			free(Ban(bot, channel, dummyuser->addr));
			nbmods++;
		    }
		    else
			nbbads++;
		    FreeCell(CELL_USERS, dummyuser);
		    if (strchr(param2,'@'))
			strcpy(op_string, param2);
		    else
			sprintf(op_string,"*@%s", param2);
		    nbmods = 0;
		    for (linklist = bot->who_list; linklist; linklist = linklist->next) {
			if (Match(((who *)linklist->cell)->chan, channel)
			   && MatchAddr(((who *)linklist->cell)->addr, op_string)) {
			    dummyuser = CreateUser(((who *)linklist->cell)->addr, channel, US_KPRO);
			    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
			    FreeCell(CELL_USERS, dummyuser);
			    if (!dummylist && 
			       strcmp(((who *)linklist->cell)->nick,((nicks *)bot->nick_list->cell)->nick)) {
				nbmods++;
				sprintf(send_socket_buffer,"%d) (%s", nbmods,param3);
				Kick(bot, channel, ((who *)linklist->cell)->nick, send_socket_buffer);
			    }
			}
		    }
		    break;
		}
	    }

	    if (!strcmp(param1,"tkb")) {
		/*
		 * param2: nick/addr
		 * param4: time
		 * param3: reason
		 */
		
		act++;
		param4 = param2;
		while ((*param4!=' ') && *param4)
		    param4++;
		if (*param4)
		    *param4++=0;
		
		param3 = param4;
		while ((*param3!=' ') && *param3)
		    param3++;
		if (*param3)
		    *param3++=0;
				
		req_time = atoi(param4); 
		
		if (req_time > 86400)   /* not longer than 1 day */
		    req_time = 86400;
		
		if (strchr(param2,'*')||strchr(param2,'@')||strchr(param2,'.')) {
		    if (strchr(param2,'@'))
			dummyuser = CreateUser(param2, channel, US_BPRO);
		    else {
			sprintf(ban_string, "*@%s", param2);
			dummyuser = CreateUser(ban_string, channel, US_BPRO);
		    }
		    dummylist = FindCell(&bot->us_list, dummyuser, FindBanUser);
		    if (!dummylist) {
			banned_addr = Ban(bot, channel, dummyuser->addr);
			sprintf(send_socket_buffer,"MoDe %s -b *!%s\n", channel, banned_addr);
			dummytodo = CreateTodo(send_socket_buffer, 0);
			dummylist = FindCell(&bot->todo_list, dummytodo, FindTodo);
			FreeCell(CELL_TODOS, dummytodo);
			if (dummylist) {
			    ((todos *)dummylist->cell)->when = time((time_t *)NULL);
			    ((todos *)dummylist->cell)->todo_time = req_time;
			} else
			    AddCellFirst(&bot->todo_list,CELL_TODOS,
					 CreateTodo(send_socket_buffer, req_time));
			free(banned_addr);
			nbmods++;
		    } else
			nbbads++;
		    FreeCell(CELL_USERS, dummyuser);
		    if (strchr(param2,'@'))
			strcpy(op_string, param2);
		    else
			sprintf(op_string,"*@%s", param2);
		    nbmods = 0;
		    for (linklist = bot->who_list; linklist; linklist = linklist->next) {
			if (Match(((who *)linklist->cell)->chan, channel)
			   && MatchAddr(((who *)linklist->cell)->addr, op_string)) {
			    dummyuser = CreateUser(((who *)linklist->cell)->addr, channel, US_KPRO);
			    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
			    FreeCell(CELL_USERS, dummyuser);
			    if (!dummylist && 
			       strcmp(((who *)linklist->cell)->nick,((nicks *)bot->nick_list->cell)->nick)) {
				nbmods++;
				sprintf(send_socket_buffer,"%d) (%s", nbmods,param3);
				Kick(bot, channel, ((who *)linklist->cell)->nick, send_socket_buffer);
			    }
			}
		    }
		    break;
		}
	    }

	    if (!strcmp(param1,"k")) {
		act++;
		if (strchr(param2,'*')) {
		    param3 = param2;
		    while ((*param3!=' ') && *param3)
			param3++;
		    if (*param3)
			*param3++=0;
		    if (strchr(param2,'@'))
			strcpy(op_string, param2);
		    else
			sprintf(op_string,"*@%s", param2);
		    nbmods = 0;
		    for (linklist = bot->who_list; linklist; linklist = linklist->next) {
			if (Match(((who *)linklist->cell)->chan, channel)
			   && MatchAddr(((who *)linklist->cell)->addr, op_string)) {
			    dummyuser = CreateUser(((who *)linklist->cell)->addr, channel, US_KPRO);
			    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
			    FreeCell(CELL_USERS, dummyuser);
			    if (!dummylist && 
			       strcmp(((who *)linklist->cell)->nick,((nicks *)bot->nick_list->cell)->nick)) {
				nbmods++;
				sprintf(send_socket_buffer,"%d) (%s", nbmods,param3);
				Kick(bot, channel, ((who *)linklist->cell)->nick, send_socket_buffer);
			    }
			}
		    }
		    break;
		}
	    }
	    
	    /**
	     * flush the log of the bot. Not done automatically
	     * to get full speed if a flood attack occur along
	     * with a takeover attempt
	     */
	    
	    if (!strcmp(param1,"seelog")) {
		act++;
		fflush(bot->outfile);
		for (linklist = bot->chan_list; linklist; linklist = linklist->next) {
		    if (((chans *)linklist->cell)->log)
			fflush(((chans *)linklist->cell)->log);
		}
		break;
	    }
	    
	    /**
	     * set the time (in seconds) between nick changes
	     * -1 means eternity (never change)
	     */

	    if (!strcmp(param1,"loopnick")) {
		act++;
		param3 = param2;
		while ((*param3!=' ') && *param3)
		    param3++;
		if (*param3)
		    *param3++=0;

		req_time = atoi(param2);
		if (req_time < 0)
		    req_time = ETERNITY;
		if (req_time < 10)
		    req_time = 10;
		bot->loop_nick_time = req_time;
		break;
	    }
	    
	    /**
	     * set the time (in second) between the arrival
	     * on the channel and the mode change (according
	     * to the preferred modes for this chan).
	     * -1 means eternity (never change)
	     */

	    if (!strcmp(param1,"redomode")) {
		act++;
		param3 = param2;
		while ((*param3!=' ') && *param3)
		    param3++;
		if (*param3)
		    *param3++=0;

		req_time = atoi(param2);
		if (req_time < 0)
		    req_time = ETERNITY;
		if (req_time < 10)
		    req_time = 10;
		bot->redo_mode_time = req_time;
		break;
	    }

	    /**
	     * set the verbosity level of the bot (0 = off)
	     */

	    if (!strcmp(param1,"verbose")) {
		act++;
		param3 = param2;
		while ((*param3!=' ') && *param3)
		    param3++;
		if (*param3)
		    *param3++=0;

		req_time = atoi(param2);
		bot->verbose = (req_time!=0);
		Notice(bot, nick, addr, (bot->verbose) ? "Verbose ON" : "Verbose OFF");
		break;
	    }

	    /**
	     * Dump the status of the bot in an HTML file
	     * usually debug.html
	     */

	    if (!strcmp(param1,"debug")) {
		act++;
		Debug(bot);
		break;
	    }
	    
	    /**
	     * kick someone randomly from the current channel
	     * protected people are avoided.
	     * This is really stupid :)
	     */

	    if (!strcmp(param1, "roulette")) {
		act++;
		num=0;
		if (bot->who_list) {
		    for (linklist = bot->who_list; linklist; linklist = linklist->next) 
			num += !strcmp(((who *)linklist->cell)->chan, channel);
		    if (num) {
			for (linklist = bot->who_list;strcmp(((who *)linklist->cell)->chan, channel);)
			    linklist = linklist->next;
			for (i = rand()%num; i; i--) {
			    linklist = linklist->next;
			    while (strcmp(((who *)linklist->cell)->chan, channel))
				linklist = linklist->next;
			}
			dummyuser = CreateUser(((who *)linklist->cell)->addr, channel, US_KPRO);
			dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
			FreeCell(CELL_USERS, dummyuser);
			if (dummylist) {
			    for (linklist = bot->who_list;strcmp(((who *)linklist->cell)->chan, channel);)
				linklist = linklist->next;
			    for (i = rand()%num; i; i--) {
				linklist = linklist->next;
				while (strcmp(((who *)linklist->cell)->chan, channel))
				    linklist = linklist->next;
			    }
			    dummyuser = CreateUser(((who *)linklist->cell)->addr, channel, US_KPRO);
			    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
			    FreeCell(CELL_USERS, dummyuser);
			}
			if (!dummylist)
			    Kick(bot, channel, ((who *)linklist->cell)->nick, "Roulette!");
		    }
		}
		break;
	    }
	    
	    case US_USER:
	    if (!strcmp(param1,"k")) {
		act++;
		param3 = param2;
		while ((*param3!=' ') && *param3)
		    param3++;
		if (*param3)
		    *param3++=0;
		dummywho = CreateWho(param2, NULL, channel);
		dummylist = FindCell(&bot->who_list, dummywho, FindWhoChan);
		FreeCell(CELL_WHO,dummywho);
		if (dummylist) {
		    dummyuser = CreateUser(((who *)dummylist->cell)->addr, channel, US_KPRO);
		    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
		    FreeCell(CELL_USERS, dummyuser);
		    if (!dummylist) {
			Kick(bot, channel, param2, param3);
			nbmods++;
		    }
		    else
			nbmods--;
		}
		break;
	    }

	    if (!strcmp(param1,"kb")) {
		act++;
		if (!param3) {
		    param3 = param2;
		    while ((*param3!=' ') && *param3)
			param3++;
		    if (*param3)
			*param3++=0;
		}
		dummywho = CreateWho(param2, NULL, channel);
		dummylist = FindCell(&bot->who_list, dummywho, FindWhoChan);
		FreeCell(CELL_WHO,dummywho);
		if (dummylist) {
		    dummyuser = CreateUser(((who *)dummylist->cell)->addr, channel, US_BPRO);
		    linklist = FindCell(&bot->us_list, dummyuser, FindUser);
		    FreeCell(CELL_USERS, dummyuser);
		    if (!linklist) {
			free(BanDop(bot, channel, param2, ((who *)dummylist->cell)->addr));
			nbmods++;
		    } else
			nbbads++;
		    dummyuser = CreateUser(((who *)dummylist->cell)->addr, channel, US_KPRO);
		    linklist = FindCell(&bot->us_list, dummyuser, FindUser);
		    FreeCell(CELL_USERS, dummyuser);
		    if (!linklist) {
			Kick(bot, channel, param2, param3);
			nbmods++;
		    } else
			nbbads++;
		} else  {
		    AddCellLast(&bot->uh_request, CELL_UH, CreateUh(C_BAN, param2, channel, 0));
		    sprintf(send_socket_buffer, "USERHOST %s\n",param2);
		    SendSocket(bot->socknum, send_socket_buffer);
		}
		break;
	    }

	    if (!strcmp(param1,"tkb")) {
		act++;
		req_time = 86400;

		if (!param3) {
		    param4 = param2;
		    while ((*param4!=' ') && *param4)
			param4++;
		    if (*param4)
			*param4++=0;
		    
		    param3 = param4;
		    while ((*param3!=' ') && *param3)
			param3++;

		    if (*param3)
			*param3++=0;
		    
		    req_time = atoi(param4);
		}
		
		if (req_time > 86400)   /* not longer than 1 day */
		    req_time = 86400;
		
		dummywho = CreateWho(param2, NULL, channel);
		dummylist = FindCell(&bot->who_list, dummywho, FindWhoChan);
		FreeCell(CELL_WHO,dummywho);
		if (dummylist) {
		    dummyuser = CreateUser(((who *)dummylist->cell)->addr, channel, US_BPRO);
		    linklist = FindCell(&bot->us_list, dummyuser, FindUser);
		    FreeCell(CELL_USERS, dummyuser);
		    if (!linklist) {
			banned_addr = BanDop(bot, channel, param2, ((who *)dummylist->cell)->addr);
			sprintf(send_socket_buffer,"MoDe %s -b *!%s\n", channel, banned_addr);
			dummytodo = CreateTodo(send_socket_buffer, 0);
			linklist = FindCell(&bot->todo_list, dummytodo, FindTodo);
			FreeCell(CELL_TODOS, dummytodo);
			if (linklist) {
			    ((todos *)linklist->cell)->when = time((time_t *)NULL);
			    ((todos *)linklist->cell)->todo_time = req_time;
			} else
			    AddCellFirst(&bot->todo_list,CELL_TODOS,
					 CreateTodo(send_socket_buffer, req_time));
			free(banned_addr);
			nbmods++;
		    } else
			nbbads++;
		    dummyuser = CreateUser(((who *)dummylist->cell)->addr, channel, US_KPRO);
		    linklist = FindCell(&bot->us_list, dummyuser, FindUser);
		    FreeCell(CELL_USERS, dummyuser);
		    if (!linklist) {
			Kick(bot, channel, param2, param3);
			nbmods++;
		    } else
			nbbads++;
		} else {
		    AddCellLast(&bot->uh_request, CELL_UH, CreateUh(C_BAN, param2, channel, 0));
		    sprintf(send_socket_buffer, "USERHOST %s\n",param2);
		    SendSocket(bot->socknum, send_socket_buffer);
		}
		break;
	    }

	    if (!strcmp(param1,"ban")) {
		act++;
		param3 = param2;
		while ((*param3!=' ') && *param3)
		    param3++;
		if (*param3)
		    *param3++=0;
		
		if (strchr(param2,'*')||strchr(param2,'@')||strchr(param2,'.')) {
		    if (strchr(param2,'@'))
			dummyuser = CreateUser(param2, channel, US_BPRO);
		    else {
			sprintf(ban_string, "*@%s", param2);
			dummyuser = CreateUser(ban_string, channel, US_BPRO);
		    }
		    dummylist = FindCell(&bot->us_list, dummyuser, FindBanUser);
		    if (!dummylist) {
			free(Ban(bot, channel, dummyuser->addr));
			nbmods++;
		    } else
			nbbads++;
		    FreeCell(CELL_USERS, dummyuser);
		} else {
		    dummywho = CreateWho(param2, NULL, channel);
		    dummylist = FindCell(&bot->who_list, dummywho, FindWhoChan);
		    FreeCell(CELL_WHO,dummywho);
		    if (dummylist) {
			dummyuser = CreateUser(((who *)dummylist->cell)->addr, channel, US_BPRO);
			dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
			if (!dummylist) {
			    free(BanDop(bot, channel, param2, dummyuser->addr));
			    nbmods++;
			} else
			    nbbads++;
			FreeCell(CELL_USERS, dummyuser);
		    } else {
			AddCellLast(&bot->uh_request, CELL_UH, CreateUh(C_BAN, param2, channel, 0));
			sprintf(send_socket_buffer, "USERHOST %s\n",param2);
			SendSocket(bot->socknum, send_socket_buffer);
		    }
		}
	    }

	    if (!strcmp(param1,"tban")) {
		act++;
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
		
		req_time = atoi(param3);
		
		if (req_time > 86400)   /* not longer than 1 day */
		    req_time = 86400;
		
		if (strchr(param2,'*')||strchr(param2,'@')||strchr(param2,'.')) {
		    if (strchr(param2,'@'))
			dummyuser = CreateUser(param2, channel, US_BPRO);
		    else {
			sprintf(ban_string, "*@%s", param2);
			dummyuser = CreateUser(ban_string, channel, US_BPRO);
		    }
		    dummylist = FindCell(&bot->us_list, dummyuser, FindBanUser);
		    if (!dummylist) {
			banned_addr = Ban(bot, channel, dummyuser->addr);
			sprintf(send_socket_buffer,"MoDe %s -b *!%s\n", channel, banned_addr);
			dummytodo = CreateTodo(send_socket_buffer, 0);
			dummylist = FindCell(&bot->todo_list, dummytodo, FindTodo);
			FreeCell(CELL_TODOS, dummytodo);
			if (dummylist) {
			    ((todos *)dummylist->cell)->when = time((time_t *)NULL);
			    ((todos *)dummylist->cell)->todo_time = req_time;
			}
			else
			    AddCellFirst(&bot->todo_list,CELL_TODOS,
					 CreateTodo(send_socket_buffer, req_time));
			free(banned_addr);
			nbmods++;
		    } else
			nbbads++;
		    FreeCell(CELL_USERS, dummyuser);
		} else {
		    dummywho = CreateWho(param2, NULL, channel);
		    dummylist = FindCell(&bot->who_list, dummywho, FindWhoChan);
		    FreeCell(CELL_WHO,dummywho);
		    if (dummylist) {
			dummyuser = CreateUser(((who *)dummylist->cell)->addr, channel, US_BPRO);
			dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
			if (!dummylist) {
			    banned_addr = BanDop(bot, channel, param2, dummyuser->addr);
			    sprintf(send_socket_buffer,"MoDe %s -b *!%s\n", channel, banned_addr);
			    dummytodo = CreateTodo(send_socket_buffer, 0);
			    dummylist = FindCell(&bot->todo_list, dummytodo, FindTodo);
			    FreeCell(CELL_TODOS, dummytodo);
			    if (dummylist) {
				((todos *)dummylist->cell)->when = time((time_t *)NULL);
				((todos *)dummylist->cell)->todo_time = req_time;
			    } else
				AddCellFirst(&bot->todo_list,CELL_TODOS,
					     CreateTodo(send_socket_buffer, req_time));
			    free(banned_addr);
			    nbmods++;
			}
			else
			    nbbads++;
			FreeCell(CELL_USERS, dummyuser);
		    } else {
			AddCellLast(&bot->uh_request, CELL_UH, CreateUh(C_TBAN, param2, channel, req_time));
			sprintf(send_socket_buffer, "USERHOST %s\n",param2);
			SendSocket(bot->socknum, send_socket_buffer);
		    }
		}
		break;
	    }
	    
	    if (!strcmp(param1,"db")) {
		act++;
		param3 = param2;
		while ((*param3!=' ') && *param3)
		    param3++;
		if (*param3)
		    *param3++=0;
		
		if (strchr(param2,'*')||strchr(param2,'@')||strchr(param2,'.')) {
		    if (strchr(param2,'@'))
			AddCellLast(&bot->db_request, CELL_BAN, CreateBan(channel, param2));
		    else {
			sprintf(ban_string, "*@%s", param2);
			AddCellLast(&bot->db_request, CELL_BAN, CreateBan(channel, ban_string));
		    }
		    Mode(bot, channel, "b", "");
		} else {
		    AddCellLast(&bot->uh_request, CELL_UH, CreateUh(C_DEBAN, param2, channel, 0));
		    sprintf(send_socket_buffer, "USERHOST %s\n",param2);
		    SendSocket(bot->socknum, send_socket_buffer);
		}
		break;
	    }

	    if (!strcmp(param1,"me")) {
		act++;
		if (!(the_chan->flags & KEEP_O))
		    Mode(bot, channel, "+o",nick);
		nbmods++;
		break;
	    }

	    if (!strcmp(param1,"op")) {
		act++;
		if (!(the_chan->flags & KEEP_O)) {
		    param3 = param2;
		    while (*param2 && (nbmods < 4)) {
			while ((*param3!=' ') && *param3)
			    param3++;
			if (*param3)
			    *param3++=0;
			
			dummywho = CreateWho(param2, NULL, channel);
			dummylist = FindCell(&bot->who_list, dummywho, FindWhoChan);
			FreeCell(CELL_WHO,dummywho);
			if (dummylist) {
			    dummyuser = CreateUser(((who *)dummylist->cell)->addr, channel, US_SHIT|US_REBAN);
			    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
			    FreeCell(CELL_USERS, dummyuser);
			    if (!dummylist) {
				sprintf(send_socket_pos," %s", param2);
				send_socket_pos+=strlen(send_socket_pos);
				nbmods++;
			    }
			}
			param2=param3;
		    }
		    if (nbmods)
			Mode(bot, channel, POPSTR, send_socket_buffer+1);
		}
		break;
	    }

	    if (!strcmp(param1,"dop")) {
		act++;
		if (!(the_chan->flags & KEEP_O)) {

		    param3 = param2;
		    while (*param2 && (nbmods < 4)) {
			while ((*param3!=' ') && *param3)
			    param3++;
			if (*param3)
			    *param3++=0;
			
			dummywho = CreateWho(param2, NULL, channel);
			dummylist = FindCell(&bot->who_list, dummywho, FindWhoChan);
			FreeCell(CELL_WHO,dummywho);
			if (dummylist) {
			    dummyuser = CreateUser(((who *)dummylist->cell)->addr, channel, US_OPRO);
			    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
			    FreeCell(CELL_USERS, dummyuser);
			    if (!dummylist) {
				sprintf(send_socket_pos," %s", param2);
				send_socket_pos+=strlen(send_socket_pos);
				nbmods++;
			    }
			}
			param2=param3;
		    }
		    if (nbmods)
			Mode(bot, channel, MOPSTR, send_socket_buffer+1);
		}
		break;
	    }

	    if (!strcmp(param1,"topic")) {
		act++;
		if (!(the_chan->flags & LOCK_TOPIC)) {
		    sprintf(send_socket_buffer,"TOPIC %s :%s\n", channel, param2);
		    SendSocket(bot->socknum, send_socket_buffer);
		}
		break;
	    }

	    if (!strcmp(param1,"whois")) {
		act++;
		param3 = param2;
		while ((*param3!=' ') && *param3)
		    param3++;
		if (*param3)
		    *param3++=0;
		
		if (!*param3)
		    param3 = channel;

		dummywho = CreateWho(param2, NULL, param3);
		dummylist = FindCell(&bot->who_list, dummywho, FindWho);
		FreeCell(CELL_WHO,dummywho);
		if (dummylist) {
		    flags = 0;
		    dummyuser = CreateUser(((who *)dummylist->cell)->addr, param3,
					   US_MASTER|US_FRIEND|US_USER|US_OPRO|US_KPRO|US_BPRO|US_AUTOOP);
		    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
		    while (dummylist) {
			flags |= ((users *)dummylist->cell)->flags;
			dummylist = dummylist->next;
			if (dummylist)
			    dummylist = FindCell(&dummylist, dummyuser, FindUser);
		    }
		    FreeCell(CELL_USERS, dummyuser);
		    if (flags) {
			sprintf(send_socket_pos,"%s is", param2);
			send_socket_pos+=strlen(send_socket_pos);
			if (flags & US_MASTER) {
			    strcpy(send_socket_pos," Beloved Master");
			    send_socket_pos+=strlen(send_socket_pos);
			}
			if (flags & US_FRIEND) {
			    strcpy(send_socket_pos," Friend");
			    send_socket_pos+=strlen(send_socket_pos);
			}
			if (flags & US_USER) {
			    strcpy(send_socket_pos," User");
			    send_socket_pos+=strlen(send_socket_pos);
			}
			if (flags & US_BOT) {
			    strcpy(send_socket_pos," Bot");
			    send_socket_pos+=strlen(send_socket_pos);
			}
			if (flags & US_AUTOOP) {
			    strcpy(send_socket_pos," Auto-Op");
			    send_socket_pos+=strlen(send_socket_pos);
			}
			if (flags & US_OPRO) {
			    strcpy(send_socket_pos," Op-pro");
			    send_socket_pos+=strlen(send_socket_pos);
			}
			if (flags & US_KPRO) {
			    strcpy(send_socket_pos," K-pro");
			    send_socket_pos+=strlen(send_socket_pos);
			}
			if (flags & US_BPRO) {
			    strcpy(send_socket_pos," Ban-pro");
			    send_socket_pos+=strlen(send_socket_pos);
			}
			sprintf(send_socket_pos," on %s", param3);
			Notice(bot, nick, addr, send_socket_buffer);
		    } else {
			sprintf(send_socket_pos,"%s is nothing on %s", param2, param3);
			Notice(bot, nick, addr, send_socket_buffer);
		    }
		    break;
		} else {
		    AddCellLast(&bot->uh_request, CELL_UH, CreateUh(C_WHOIS, nick, param3, 0));
		    sprintf(send_socket_buffer, "USERHOST %s\n",param2);
		    SendSocket(bot->socknum, send_socket_buffer);
		}
		break;
	    }

	    if (!strcmp(param1, "nsl")) {
		act++;
		param3 = param2;
		while ((*param3!=' ') && *param3)
		    param3++;
		if (*param3)
		    *param3++=0;
		
		if (IsIp(param2)) {
		    send_socket_pos = IpToName(param2);
		    Notice(bot, nick, addr, send_socket_pos);
		    free(send_socket_pos);
		} else {
		    send_socket_pos = NameToIp(param2);
		    Notice(bot, nick, addr, send_socket_pos);
		    free(send_socket_pos);
		}
		break;
	    }

	default:
	    if (!strcmp(param1,"level")) {
		act++;
		flags = 0;
		dummyuser = CreateUser(addr, channel, US_MASTER|US_FRIEND|US_USER|US_OPRO|US_KPRO|US_BPRO|US_AUTOOP);
		dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
		while (dummylist) {
		    flags |= ((users *)dummylist->cell)->flags;
		    dummylist = dummylist->next;
		    if (dummylist)
			dummylist = FindCell(&dummylist, dummyuser, FindUser);
		}
		FreeCell(CELL_USERS, dummyuser);
		if (flags) {
		    strcpy(send_socket_pos,"You are");
		    send_socket_pos+=strlen(send_socket_pos);
		    if (flags & US_MASTER) {
			strcpy(send_socket_pos," Beloved Master");
			send_socket_pos+=strlen(send_socket_pos);
		    }
		    if (flags & US_FRIEND) {
			strcpy(send_socket_pos," Friend");
			send_socket_pos+=strlen(send_socket_pos);
		    }
		    if (flags & US_USER) {
			strcpy(send_socket_pos," User");
			send_socket_pos+=strlen(send_socket_pos);
		    }
		    if (flags & US_BOT) {
			strcpy(send_socket_pos," Bot");
			send_socket_pos+=strlen(send_socket_pos);
		    }
		    if (flags & US_AUTOOP) {
			strcpy(send_socket_pos," Auto-Op");
			send_socket_pos+=strlen(send_socket_pos);
		    }
		    if (flags & US_OPRO) {
			strcpy(send_socket_pos," Op-pro");
			send_socket_pos+=strlen(send_socket_pos);
		    }
		    if (flags & US_KPRO) {
			strcpy(send_socket_pos," K-pro");
			send_socket_pos+=strlen(send_socket_pos);
		    }
		    if (flags & US_BPRO) {
			strcpy(send_socket_pos," Ban-pro");
			send_socket_pos+=strlen(send_socket_pos);
		    }
		    sprintf(send_socket_pos," on %s", channel);
		    Notice(bot, nick, addr, send_socket_buffer);
		} else {
		    sprintf(send_socket_pos,"You are nothing on %s", channel);
		    Notice(bot, nick, addr, send_socket_buffer);
		}
		break;
	    }
	    
	    if (!strcmp(param1,"rot13")) {
		Rot13(param2);
		Notice(bot, nick, addr, param2);
		break;
	    }

	    if (!strcmp(param1,"help")) {
		act++;
		flags = 0;
		dummyuser = CreateUser(addr, channel, US_MASTER|US_FRIEND|US_USER|US_OPRO|US_KPRO|US_BPRO|US_AUTOOP);
		dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
		while (dummylist) {
		    flags |= ((users *)dummylist->cell)->flags;
		    dummylist = dummylist->next;
		    if (dummylist)
			dummylist = FindCell(&dummylist, dummyuser, FindUser);
		}
		FreeCell(CELL_USERS, dummyuser);
		sprintf(send_socket_pos,"On %s:", channel);
		send_socket_pos+=strlen(send_socket_pos);
		if (flags & US_MASTER) {
		    strcpy(send_socket_pos," keep, noop, allowop, norevenge, allowrevenge, nomode, allowmode, nohack, allowhack, startlog, stoplog, die, saveall, rehash, adduser, addshit, loopserv, userinfo, gmt, local, gateway, nogateway");
		    send_socket_pos+=strlen(send_socket_pos);
		}
		if (flags & US_FRIEND) {
		    strcpy(send_socket_pos," status, botstatus, seelog, loopnick, redomode, verbose, debug, k, kb, tkb, ban, tban, db, me, op, dop, topic, whois, leave");
		    send_socket_pos+=strlen(send_socket_pos);
		} else
		    if (flags & US_USER) {
			strcpy(send_socket_pos," k, kb, tkb, ban, tban, db, me, op, dop, topic, whois,");
			send_socket_pos+=strlen(send_socket_pos);
		    }
		strcpy(send_socket_pos," level, help, rot13");
		Notice(bot, nick, addr, send_socket_buffer);
	    }
	    break;
	}

	if (the_user) {
	    the_user->actions += nbmods + act;
	    the_user->bad_actions += nbbads;
	}
	if (act)
	    fputs(command, bot->outfile);
    }
}

void ClientParsePrivate(bot, nick, addr, param)
    irc_bot *bot;
    char *nick, *addr, *param;
{
    char  *param1;
    char  *param2;
    char  send_socket_buffer[512];
    list  *dummylist;
    users *the_user, *dummyuser;
    todos *dummytodo;

    param1 = param+(*param==':');
    param2=param1;
    while ((*param2!=' ') && *param2)
	param2++;
    if (*param2)
	*param2++=0;

    if (*param1 == '') {
	HandleCtcp(bot,nick,addr,param1,param2);
	return;
    }
    if (bot->verbose) {
	char the_time[64];

	ShortTime(the_time);
	fprintf(bot->outfile,"[%s] *%s!%s* %s %s\n", the_time, nick, addr, param1, param2);
    }
    dummyuser = CreateUser(addr, "#*", US_MASTER|US_FRIEND);
    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
    the_user = (users *)((dummylist) ? dummylist->cell : NULL);
    if (the_user) {
	if (the_user->flags & (US_MASTER|US_FRIEND)) {
	    if (!strcmp(param1,"sign")) {
		dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
		while (dummylist) {
     /* due to the structure of userlist, we must update every matching cell */
		    UpdateUser(&dummylist, addr, "#*", US_MASTER, MASTER_TIME, FindUser);
		    dummylist = dummylist->next;
		    if (dummylist)
			dummylist = FindCell(&dummylist, dummyuser, FindUser);
		}
		sprintf(send_socket_buffer,"NOTICE %s :You are no longer my master\n", nick);
		dummytodo = CreateTodo(send_socket_buffer, 0);
		dummylist = FindCell(&bot->todo_list, dummytodo, FindTodo);
		FreeCell(CELL_TODOS, dummytodo);
		if (dummylist)
		    ((todos *)dummylist->cell)->when = time((time_t *)NULL);
		else
		    AddCellFirst(&bot->todo_list,CELL_TODOS,
				 CreateTodo(send_socket_buffer, MASTER_TIME));
		sprintf(send_socket_buffer,"Welcome Master %s. %d seconds left...", nick, MASTER_TIME);
		Notice(bot, nick, addr, send_socket_buffer);
	    } else if (!strcmp(param1,"do")) {
		if (the_user->flags & US_MASTER) {
		    sprintf(send_socket_buffer,"%s\n", param2);
		    SendSocket(bot->socknum, send_socket_buffer);
		}
	    }
	}
    }
    FreeCell(CELL_USERS, dummyuser);
}

void ClientParseUserhost(bot, result)
    irc_bot *bot;
    char *result;
{
    char *nick, *addr, *banned_addr;
    uh *the_uh;
    users *dummyuser;
    todos *dummytodo;
    list *dummylist;
    int flags, umodes, ubads;
    char send_socket_buffer[512];
    char *send_socket_pos;

    the_uh = (uh *)bot->uh_request->cell;
    
    nick = result;
    addr = result;
    
    while (*addr && (*addr!='*') && (*addr != '=') && (*addr != '-'))
	addr++;
    if (addr) {
	*addr++=0;
	while (*addr && (*addr!='+') && (*addr != '-'))
	    addr++;
	if (addr)
	    addr++;
    }
    switch(the_uh->type) {
    case C_TBAN:
	dummyuser = CreateUser(addr, the_uh->chan, US_BPRO);
	dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
	if (!dummylist) {
	    banned_addr = Ban(bot, the_uh->chan, dummyuser->addr);
	    sprintf(send_socket_buffer,"MoDe %s -b *!%s\n", the_uh->chan, banned_addr);
	    dummytodo = CreateTodo(send_socket_buffer, 0);
	    dummylist = FindCell(&bot->todo_list, dummytodo, FindTodo);
	    FreeCell(CELL_TODOS, dummytodo);
	    if (dummylist) {
		((todos *)dummylist->cell)->when = time((time_t *)NULL);
		((todos *)dummylist->cell)->todo_time = the_uh->timer;
	    } else
		AddCellFirst(&bot->todo_list,CELL_TODOS,
			     CreateTodo(send_socket_buffer, the_uh->timer));
	    free(banned_addr);
	}
	FreeCell(CELL_USERS, dummyuser);
	break;
    case C_BAN:
	dummyuser = CreateUser(addr, the_uh->chan, US_BPRO);
	dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
	if (!dummylist) {
	    banned_addr = Ban(bot, the_uh->chan, dummyuser->addr);
	    free(banned_addr);
	}
	FreeCell(CELL_USERS, dummyuser);
	break;
    case C_DEBAN:
	AddCellLast(&bot->db_request, CELL_BAN, CreateBan(the_uh->chan, addr));
	Mode(bot, the_uh->chan, "b", "");
	break;
    case C_WHOIS:
	flags = 0;
	dummyuser = CreateUser(addr, the_uh->chan,
			       US_MASTER|US_FRIEND|US_USER|US_OPRO|US_KPRO|US_BPRO|US_AUTOOP);
	dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
	while (dummylist) {
	    flags |= ((users *)dummylist->cell)->flags;
	    dummylist = dummylist->next;
	    if (dummylist)
		dummylist = FindCell(&dummylist, dummyuser, FindUser);
	}
	FreeCell(CELL_USERS, dummyuser);
	if (flags) {
	    send_socket_pos = send_socket_buffer;
	    sprintf(send_socket_pos,"%s is", nick);
	    send_socket_pos+=strlen(send_socket_pos);
	    if (flags & US_MASTER) {
		strcpy(send_socket_pos," Beloved Master");
		send_socket_pos+=strlen(send_socket_pos);
	    }
	    if (flags & US_FRIEND) {
		strcpy(send_socket_pos," Friend");
		send_socket_pos+=strlen(send_socket_pos);
	    }
	    if (flags & US_USER) {
		strcpy(send_socket_pos," User");
		send_socket_pos+=strlen(send_socket_pos);
	    }
	    if (flags & US_BOT) {
		strcpy(send_socket_pos," Bot");
		send_socket_pos+=strlen(send_socket_pos);
	    }
	    if (flags & US_AUTOOP) {
		strcpy(send_socket_pos," Auto-Op");
		send_socket_pos+=strlen(send_socket_pos);
	    }
	    if (flags & US_OPRO) {
		strcpy(send_socket_pos," Op-pro");
		send_socket_pos+=strlen(send_socket_pos);
	    }
	    if (flags & US_KPRO) {
		strcpy(send_socket_pos," K-pro");
		send_socket_pos+=strlen(send_socket_pos);
	    }
	    if (flags & US_BPRO) {
		strcpy(send_socket_pos," Ban-pro");
		send_socket_pos+=strlen(send_socket_pos);
	    }
	    sprintf(send_socket_pos," on %s", the_uh->chan);
	    Notice(bot, the_uh->nick, addr, send_socket_buffer);
	} else {
	    sprintf(send_socket_buffer,"%s nothing on %s", nick, the_uh->chan);
	    Notice(bot, the_uh->nick, addr, send_socket_buffer);
	}
	break;
    case C_USERINFO:
	flags = 0;
	umodes = 0;
	ubads = 0;
	dummyuser = CreateUser(addr, the_uh->chan,
			       US_MASTER|US_FRIEND|US_USER|US_OPRO|US_KPRO|US_BPRO|US_AUTOOP);
	dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
	while (dummylist) {
	    flags |= ((users *)dummylist->cell)->flags;
	    umodes += ((users *)dummylist->cell)->actions;
	    ubads += ((users *)dummylist->cell)->bad_actions;
	    dummylist = dummylist->next;
	    if (dummylist)
		dummylist = FindCell(&dummylist, dummyuser, FindUser);
	}
	send_socket_pos = send_socket_buffer;
	if (flags) {
	    sprintf(send_socket_pos,"%s is ", nick);
	    send_socket_pos+=strlen(send_socket_pos);
	    if (flags & US_MASTER)
		*send_socket_pos++='m';
	    if (flags & US_FRIEND)
		*send_socket_pos++='f';
	    if (flags & US_USER)
		*send_socket_pos++='u';
	    if (flags & US_BOT)
		*send_socket_pos++='b';
	    if (flags & US_AUTOOP)
		*send_socket_pos++='a';
	    if (flags & US_OPRO)
		*send_socket_pos++='o';
	    if (flags & US_KPRO)
		*send_socket_pos++='k';
	    if (flags & US_BPRO)
		*send_socket_pos++='p';
	    sprintf(send_socket_pos," action/bad %d/%d on %s", umodes, ubads, the_uh->chan);
	    Notice(bot, the_uh->nick, addr, send_socket_buffer);
	} else {
	    sprintf(send_socket_pos,"%s is nothing on %s", nick, the_uh->chan);
	    Notice(bot, the_uh->nick, addr, send_socket_buffer);
	}
	FreeCell(CELL_USERS, dummyuser);
	break;
    }
    FreeLink(&bot->uh_request);
}

void ClientParseBan(bot, chan)
    irc_bot *bot;
    char    *chan;
{
    list  *dummylist,**dummylink,**bancelladdr;
    users *dummyuser;
    ban   *the_ban, *dummyban;
    char  send_socket_buffer[512];
    char  *send_socket_pos;
    int   nb_deban;

    send_socket_pos = send_socket_buffer;
    *send_socket_pos = 0;
    *(send_socket_pos+1) = 0;
    dummyban = CreateBan(chan, NULL);
    bancelladdr = FindCellAddr(&bot->db_request, dummyban, FindBanChannel);
    the_ban = (ban *)(*bancelladdr)->cell;
    dummylink = FindCellAddr(&bot->ban_list, dummyban, FindBanChannel);
    FreeCell(CELL_BAN, dummyban);
    nb_deban=0;

    while (dummylink) {
	dummyuser = CreateUser(((ban *)(*dummylink)->cell)->addr, chan, US_REBAN|US_SHIT);
	dummylist = FindCell(&bot->us_list, dummyuser, FindBanUser);
	FreeCell(CELL_USERS, dummyuser);
	if (!dummylist) {
	    dummyban = CreateBan(chan, ((ban *)(*dummylink)->cell)->addr);
	    if (FindBan(the_ban, dummyban)) {
		sprintf(send_socket_pos," *!%s", ((ban *)(*dummylink)->cell)->addr);
		send_socket_pos += strlen(send_socket_pos);
		nb_deban++;
		if (nb_deban == 3) {
		    Mode(bot, chan, "-bbb", send_socket_buffer+1);
		    nb_deban = 0;
		    send_socket_pos = send_socket_buffer;
		}
	    }
	    FreeCell(CELL_BAN, dummyban);
	}
	FreeLink(dummylink);
	dummylink = FindCellAddr(&bot->ban_list, the_ban, FindBanChannel);
    }
    if (nb_deban == 2)
	Mode(bot, chan, "-bb", send_socket_buffer+1);
    else 
	if (nb_deban == 1)
	    Mode(bot, chan, "-b", send_socket_buffer+1);
    FreeLink(bancelladdr);
}
