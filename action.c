/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include "types.h"
#include "action.h"
#include "socket.h"
#include "strip.h"
#include "time.h"

void Pong(bot, ping_msg)
    irc_bot *bot;
    char *ping_msg;
{
    char *send_buffer;
    
    send_buffer = (char *)malloc(strlen(ping_msg)+7);
    sprintf(send_buffer,"PONG %s\n",ping_msg);
    if (SendSocket(bot->socknum, send_buffer) < 1) {
	char the_time[64];

	ShortTime(the_time);
	close(bot->socknum);
	bot->state = STATE_NOT_CONNECTED;
	fprintf(bot->outfile, "*** WRITE ERROR [%s] in PONG\n", the_time);
    }
    free(send_buffer);
}

void Join(bot, chan)
    irc_bot *bot;
    char *chan;
{
    char *send_buffer;
    
    send_buffer = (char *)malloc(strlen(chan)+7);
    sprintf(send_buffer,"JOIN %s\n",chan);
    if (SendSocket(bot->socknum, send_buffer) < 1) {
	char the_time[64];

	ShortTime(the_time);
	close(bot->socknum);
	bot->state = STATE_NOT_CONNECTED;
	fprintf(bot->outfile, "*** WRITE ERROR [%s] in JOIN\n", the_time);
    }
    bot->last_action = ACTION_CHAN;
    free(send_buffer);
}

void Notice(bot, towho, addr, what)
    irc_bot *bot;
    char *towho,*addr,*what;
{
    char *send_buffer;
    
    if (bot->nb_notice < MAX_NOTICES) {
	send_buffer = (char *)malloc(strlen(towho)+strlen(what)+11);
	sprintf(send_buffer,"NOTICE %s :%s\n",towho,what);
	if (SendSocket(bot->socknum, send_buffer) < 1) {
	    char the_time[64];

	    ShortTime(the_time);
	    close(bot->socknum);
	    bot->state = STATE_NOT_CONNECTED;
	    fprintf(bot->outfile,"*** WRITE ERROR [%s] in NOTICE\n", the_time);
	}
	free(send_buffer);
    } else {
	char the_time[64];

	ShortTime(the_time);
	if (bot->verbose)
	    printf("-> dropping Notice for %s [%s]\n", towho, what);
	fprintf(bot->outfile, "*** [%s] FLOOD ATTACK from %s!%s [%s]\n", 
		the_time, towho, addr, what);
    }
    ++bot->nb_notice;
}

void PrivMsg(bot, towho, what)
    irc_bot *bot;
    char *towho,*what;
{
    char *send_buffer;
    
    send_buffer = (char *)malloc(strlen(towho)+strlen(what)+12);
    sprintf(send_buffer,"PRIVMSG %s :%s\n",towho,what);
    if (SendSocket(bot->socknum, send_buffer) < 1) {
	char the_time[64];
 
	ShortTime(the_time);
	close(bot->socknum);
	bot->state = STATE_NOT_CONNECTED;
	fprintf(bot->outfile, "*** WRITE ERROR [%s] in PRIVMSG\n", the_time);
    }
    free(send_buffer);
}

void Mode(bot, chan, mode, param)
    irc_bot *bot;
    char *chan,*mode,*param;
{
    char *send_buffer;
    
    send_buffer = (char *)malloc(strlen(chan)+strlen(mode)+strlen(param)+9);
    sprintf(send_buffer,"MODE %s %s %s\n", chan, mode, param);
    if (SendSocket(bot->socknum, send_buffer) < 1) {
	char the_time[64];

	ShortTime(the_time);
	close(bot->socknum);
	bot->state = STATE_NOT_CONNECTED;
	fprintf(bot->outfile, "*** WRITE ERROR [%s] in MODE\n", the_time);
    }
    free(send_buffer);
}

void Kick(bot, chan, towho, reason)
    irc_bot *bot;
    char *chan,*towho,*reason;
{
    char *send_buffer;
    
    send_buffer = (char *)malloc(strlen(chan)+strlen(towho)+strlen(reason)+10);
    sprintf(send_buffer,"KICK %s %s :%s\n", chan, towho, reason);
    if (SendSocket(bot->socknum, send_buffer) < 1) {
	char the_time[64];

	ShortTime(the_time);
	close(bot->socknum);
	bot->state = STATE_NOT_CONNECTED;
	fprintf(bot->outfile, "*** WRITE ERROR [%s] in KICK\n", the_time);
    }
    free(send_buffer);
}

char *Ban(bot, channel, addr)
    irc_bot *bot;
    char *channel, *addr;
{
    char *banned_addr;
    char send_socket_buffer[512];

    banned_addr = (char *)malloc(strlen(addr)+16);
    if (IsNumeric(addr))
	StripNumeric(addr, banned_addr);
    else {
	if (!strchr(addr,'*'))
	    Strip(addr, banned_addr);
	else
	    strcpy(banned_addr, addr);
    }
    sprintf(send_socket_buffer,"*!%s", banned_addr);
    Mode(bot, channel, "+b", send_socket_buffer);
    return banned_addr;
}

char *BanDop(bot, channel,nick, addr)
    irc_bot *bot;
    char *channel, *nick, *addr;
{
    char *banned_addr;
    char send_socket_buffer[512];
    
    banned_addr = (char *)malloc(strlen(addr)+16);
    if (IsNumeric(addr))
	StripNumeric(addr, banned_addr);
    else {
	if (!strchr(addr,'*'))
	    Strip(addr, banned_addr);
	else
	    strcpy(banned_addr, addr);
    }
    sprintf(send_socket_buffer,"%s *!%s", nick, banned_addr);
    Mode(bot, channel, "-o+b", send_socket_buffer);
    return(banned_addr);
}
