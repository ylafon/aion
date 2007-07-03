/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <malloc.h>
#include "types.h"
#include "ctcp.h"
#include "socket.h"
#include "time.h"
#include "action.h"
#include "version.h"

void HandleCtcp(bot, nickname, addr, command, body)
     irc_bot *bot;
     char *nickname, *addr, *command, *body;
{
    char buffer[512];
    char tmp[256];
    int sec,mn,hr,day;
    int masterclock;
    struct utsname os_info; 
    
    *buffer = 0;

    if (!strcmp(command,"PING")) {
	sprintf(buffer, "PING %s", body);
	Notice(bot, nickname, addr, buffer);
	return;
    }
    if (!strcmp(command,"PING")) {
	Notice(bot, nickname, addr, "PING");
	return;
    }
    if (!strcmp(command,"COMMAND")) {
	sprintf(buffer,"COMMAND %c", bot->cmd_char);
	Notice(bot, nickname, addr, buffer);
	return;
    }
    if (!strcmp(command,"VERSION")) {
	sprintf(buffer,"VERSION %s v%d.%d" AION_AUTHOR ": still under construction, sorry for the bugs ;)",
		((nicks *)bot->nick_list->cell)->nick, AION_MAJOR_VERSION, AION_MINOR_VERSION);
	Notice(bot, nickname, addr, buffer);
	return;
    }
    if (!strcmp(command,"FINGER")) {
	uname(&os_info);
	sprintf(buffer,"FINGER %s (%s@%s) Idle %d seconds", bot->realname, bot->username, 
		os_info.nodename, (int)difftime(time((time_t *)NULL), bot->uptime));
	Notice(bot, nickname, addr, buffer);
	return;
    }
    if (!strcmp(command,"CLOCK")) {
	masterclock = difftime(time((time_t *)NULL),bot->uptime);
 	sec=masterclock%60;
	mn=((masterclock-sec)/60)%60;
	hr=((masterclock-sec-mn*60)/3600)%24;
	day=(masterclock-sec-mn*60-hr*3600)/86400;
	sprintf(buffer,"CLOCK elapsed time : %dd%dh%d:%d", day, hr, mn, sec);
	Notice(bot, nickname, addr, buffer);
	return;
    }
    if (!strcmp(command,"TIME")) {
	LocalTime(tmp);
	sprintf(buffer,"TIME %s",tmp);
	Notice(bot, nickname, addr, buffer);
	return;
    }
    if (!strcmp(command,"INFO")||!strcmp(command,"CLIENTINFO")) {
	sprintf(buffer,"INFO Available CTCP: TIME INFO CLIENTINFO CLOCK COMMAND FINGER VERSION PING");
	Notice(bot, nickname, addr, buffer);
	return;
    }
}
