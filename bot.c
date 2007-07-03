/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: bot.c,v 1.2 2007/07/03 14:02:57 ylafon Exp $ */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netdb.h>
#include <time.h>
#include "types.h"
#include "defs.h"
#include "bot.h"
#include "socket.h"
#include "list.h"

irc_bot *CreateBot()
{
    irc_bot *newbot;

    newbot = (irc_bot *) malloc(sizeof(irc_bot));

    if (!newbot)
	return newbot;

    newbot->state          = STATE_NOT_CONNECTED;
    newbot->cmd_char       = BOT_COMMAND_CHAR;
    newbot->sockbufferpos  = newbot->sockbuffer;
    newbot->quit           = FALSE;
    newbot->verbose        = TRUE;
    newbot->nb_notice      = 0;
    newbot->last_action    = ACTION_NICK;
    newbot->outfile        = 0;
    newbot->uptime         = time((time_t *)NULL);
    newbot->config_flag    = 0;
    newbot->username       = NULL;
    newbot->realname       = NULL;
    newbot->behaviour      = B_CLIENT|B_LOOP_ON_ERROR|B_LOOP_SERV_ON_COLL|\
	B_LOOP_NICK_ON_COLL;
    newbot->hack_max_time  = HACK_MAX_TIME;
    newbot->max_ping       = MAX_PING;
    newbot->revenge_time   = REVENGE_TIME;
    newbot->lame_fake_time = LAME_FAKE_TIME;
    newbot->loop_nick_time = LOOP_NICK_TIME;
    newbot->redo_mode_time = REDO_MODE_TIME;
    newbot->hack_reop_time = HACK_REOP_TIME;
    newbot->rejoin_time    = REJOIN_TIME;
    newbot->war_time       = WAR_TIME;
    newbot->host_list      = NULL;
    newbot->nick_list      = NULL;
    newbot->us_list        = NULL;
    newbot->war_list       = NULL;
    newbot->who_list       = NULL;
    newbot->db_request     = NULL;
    newbot->uh_request     = NULL;
    newbot->ban_list       = NULL;
    newbot->chan_list      = NULL;
    newbot->todo_list      = NULL;
    newbot->hack_list      = NULL;
    return(newbot);
}

void ConnectBot(bot)
    irc_bot *bot;
{
    bot->socknum = Connection(((hosts *)bot->host_list->cell)->name,((hosts *)bot->host_list->cell)->port);
    bot->state = (bot->socknum) ? STATE_NOT_REGISTERED : STATE_NOT_CONNECTED;
}

void RegisterBot(bot)
    irc_bot *bot;
{
    char reg_buffer[512];
    char *passwd;
    struct hostent *host_info;
    struct utsname os_info;

    uname(&os_info);
    host_info = gethostbyname(os_info.nodename);
    passwd = ((hosts *)bot->host_list->cell)->passwd;
    if (passwd) {
	sprintf(reg_buffer,"PASS %s\r\nUSER %s %s %s :%s\r\nNICK %s\r\n",
		passwd, bot->username, host_info->h_name,
		((hosts *)bot->host_list->cell)->name, bot->realname,
		((nicks *)bot->nick_list->cell)->nick);
    } else {
	sprintf(reg_buffer,"USER %s %s %s :%s\r\nNICK %s\r\n",
		bot->username, host_info->h_name,
		((hosts *)bot->host_list->cell)->name, bot->realname,
		((nicks *)bot->nick_list->cell)->nick);
    }
    if (SendSocket(bot->socknum, reg_buffer)) {
	bot->state       = STATE_RUNNING;
	bot->last_action = ACTION_NICK;
    } else {
	if (bot->verbose)
	    printf("exiting\n");
	bot->state = STATE_NOT_CONNECTED;
	shutdown(bot->socknum, 2);
	close(bot->socknum);
	LoopList(&bot->host_list); 

    }
    bot->s_ping = time((time_t *)NULL);
    ((nicks *)bot->nick_list->cell)->uptime = bot->s_ping;
}

void ChangeNick(bot)
    irc_bot *bot;
{
    char nick_buffer[256];

    LoopList(&bot->nick_list);
    sprintf(nick_buffer,"NICK %s\n",((nicks *)(bot->nick_list->cell))->nick);
    SendSocket(bot->socknum, nick_buffer);
    ((nicks *)bot->nick_list->cell)->uptime = time((time_t *)NULL);
    bot->last_action = ACTION_NICK;
} 

void FreeBot(bot)
    irc_bot *bot;
{
    if (bot) {
	FreeList(&bot->host_list);
	FreeList(&bot->nick_list);
	FreeList(&bot->us_list);
	FreeList(&bot->war_list);
	FreeList(&bot->who_list);
	FreeList(&bot->db_request);
	FreeList(&bot->uh_request);
	FreeList(&bot->ban_list);
	FreeList(&bot->chan_list);
	FreeList(&bot->todo_list);
	FreeList(&bot->hack_list);
	if (bot->realname)
	    free(bot->realname);
	if (bot->configname)
	    free(bot->configname);
	if (bot->logname)
	    free(bot->logname);
	if (bot->username)
	    free(bot->username);
	free(bot);
    }
}
    
void ShutBot(bot)
    irc_bot *bot;
{
    if (bot) {
	fclose(bot->outfile);
	if (bot->state != STATE_NOT_CONNECTED) {
	    shutdown(bot->socknum, 2);
	    close(bot->socknum);
	}
	FreeBot(bot);
    }
}
	    
    
