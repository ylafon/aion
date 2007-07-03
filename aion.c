/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#ifndef SUNOS
#include <sys/resource.h>
#endif /* !SUNOS */
#include "types.h"
#include "bot.h"
#include "list.h"
#include "mode.h"
#include "cell.h"
#include "socket.h"
#include "parse.h"
#include "compare.h"
#include "file.h"
#include "signal.h"
#include "time.h"


/**
 * small note: this file is supposed to be ugly!
 * If you can still read it, it may be a bug ;)
 */

int main(argc, argv)
    int argc;
    char **argv;
{
    list *bot_list, *tmp_link, *next_link, *chan_link, *tmp_l;
    irc_bot *bot;
    int max_fd;
    char *parse_pos, filename[64];
    char *last_parsed, the_time[64];
    char faked_event[32] = "Internal.request 999";
    int len, i, pid, is_first;
    struct timeval timeout,tv;
    struct timezone tz;
    fd_set rd,rw;
#ifndef SUNOS
    struct rlimit rlp;
#endif /* !SUNOS */
    time_t last_update, current_time;

    (void)gettimeofday(&tv,&tz);
    srand((unsigned int)tv.tv_usec); 
    bot_list = NULL;

    if (argc > 1) {
	pid = (int)getpid()*(1+(argc-1)/10)*10;
	for (i=argc-1; i; i--) {
/*
 * it can be a good thing to test if the configuration file is valid...
 */
	    bot = CreateBot();
	    AddCellFirst(&bot_list, CELL_BOT, bot);
	    bot->configname = (char *)malloc(strlen(argv[i])+1);
	    strcpy(bot->configname, argv[i]);
	    LoadAll(bot);
	    sprintf(filename,"bot%d.log", pid+i);
	    bot->outfile = fopen(filename,"w");
	    bot->logname =  (char *)malloc(strlen(filename)+1);
	    strcpy(bot->logname, filename);
	}
    } else {
	printf("You must provide at least one configuration file\n");
	exit(1);
    }

    IgnoreAll();
#ifdef SUNOS
    max_fd = getdtablesize();
#else
    getrlimit(RLIMIT_NOFILE, &rlp);
    max_fd = (int) rlp.rlim_cur;
#endif /* SUNOS */


/* 
 * first connection is only cosmetic
 * the main loop is able to handle this first connection.
 */

    for (tmp_link = bot_list; tmp_link; tmp_link=tmp_link->next)
	ConnectBot((irc_bot *)tmp_link->cell);

    current_time = time((time_t *)NULL);
    last_update = current_time;
    while (bot_list) {
	current_time = time((time_t *)NULL);

/*
 * check if 2 bots are on the same server...
 * in this case, close the one which came last on it
 */

	for (tmp_link = bot_list; tmp_link; tmp_link=tmp_link->next) {
	    for (tmp_l = tmp_link->next; tmp_l; tmp_l = tmp_l->next) {
		if (!strcmp(((hosts *)((irc_bot *)tmp_link->cell)->host_list->cell)->name, ((hosts *)((irc_bot *)tmp_l->cell)->host_list->cell)->name)) {
		    if ((((irc_bot *)tmp_link->cell)->state == STATE_RUNNING)&&(((irc_bot *)tmp_l->cell)->state == STATE_RUNNING)) {
			if (difftime(((hosts *)((irc_bot *)tmp_link->cell)->host_list->cell)->uptime, ((hosts *)((irc_bot *)tmp_l->cell)->host_list->cell)->uptime) > 0) { /* tmp_l one is younger */
			    ((irc_bot *)tmp_l->cell)->state = STATE_NOT_CONNECTED;		
			    FreeList(&((irc_bot *)tmp_l->cell)->who_list);
			    LoopList(&((irc_bot *)tmp_l->cell)->host_list);
			    shutdown(((irc_bot *)tmp_l->cell)->socknum,2);
			    close(((irc_bot *)tmp_l->cell)->socknum);
			} else {
			    ((irc_bot *)tmp_link->cell)->state = STATE_NOT_CONNECTED;		
			    FreeList(&((irc_bot *)tmp_link->cell)->who_list);
			    LoopList(&((irc_bot *)tmp_link->cell)->host_list);
			    shutdown(((irc_bot *)tmp_link->cell)->socknum,2);
			    close(((irc_bot *)tmp_link->cell)->socknum);
			}
		    }
		}
	    }
	}    
		    
	if (difftime(current_time, last_update)) { /* customizable time */
	    last_update = current_time;

	    /* check for CD/ND timeout */	
	    for (tmp_link = bot_list; tmp_link; tmp_link=tmp_link->next) {
		if (!(((irc_bot *)tmp_link->cell)->behaviour & (B_CLIENT|B_CHECK_CHANNEL_TIMEOUT)) && (((irc_bot *)tmp_link->cell)->state == STATE_RUNNING)) { 
		    if (difftime(((irc_bot *)tmp_link->cell)->join_timestamp, current_time) > CHANNEL_DELAY_TIMEOUT) {
			((irc_bot *)tmp_link->cell)->join_timestamp = current_time;
			printf("*** joinstamp reached, parsing faked event");
			Parse(((irc_bot *)tmp_link->cell), faked_event);
		    }
		}
	    }	    

	    /* purge lists */
	    for (tmp_link = bot_list; tmp_link; tmp_link=tmp_link->next) {
		PurgeTodoList((irc_bot *)tmp_link->cell, &((irc_bot *)tmp_link->cell)->todo_list);
		PurgeHackList((irc_bot *)tmp_link->cell, &((irc_bot *)tmp_link->cell)->hack_list);
		PurgeUserList(&((irc_bot *)tmp_link->cell)->us_list);
		if (((irc_bot *)tmp_link->cell)->nb_notice<MAX_NOTICES/2) {
		    if (((irc_bot *)tmp_link->cell)->nb_notice<MAX_NOTICES/4)
			((irc_bot *)tmp_link->cell)->nb_notice = 0;
		    else
			((irc_bot *)tmp_link->cell)->nb_notice -= MAX_NOTICES/4;
		}
		else
		    --((irc_bot *)tmp_link->cell)->nb_notice; 
	    }
	}

	for (tmp_link = bot_list; tmp_link; tmp_link=tmp_link->next) {
	    if (difftime(current_time, ((nicks *)((irc_bot *)tmp_link->cell)->nick_list->cell)->uptime) >
	       ((irc_bot *)tmp_link->cell)->loop_nick_time)
		if (((irc_bot *)tmp_link->cell)->state == STATE_RUNNING)
		    ChangeNick((irc_bot *)tmp_link->cell);

	    if (((irc_bot *)tmp_link->cell)->state == STATE_NOT_CONNECTED)
	    {
		((irc_bot *)tmp_link->cell)->sockbufferpos = ((irc_bot *)tmp_link->cell)->sockbuffer;
		FreeList(&((irc_bot *)tmp_link->cell)->who_list);
		ConnectBot((irc_bot *)tmp_link->cell);
		if (((irc_bot *)tmp_link->cell)->state == STATE_NOT_CONNECTED)
		    LoopList(&((irc_bot *)tmp_link->cell)->host_list);   /* no connection done -> loop */
	    } else {
		if (((irc_bot *)tmp_link->cell)->state == STATE_NOT_REGISTERED)
		    RegisterBot((irc_bot *)tmp_link->cell);
		else {
		    for (chan_link = ((list *)((irc_bot *)tmp_link->cell)->chan_list); chan_link; chan_link = chan_link->next) {
			if (((chans *)chan_link->cell)->flags & FORCE_MODE) {
			    if (difftime(current_time, ((chans *)chan_link->cell)->last_mode) > 
			       ((irc_bot *)tmp_link->cell)->redo_mode_time) {
				i = ((chans *)chan_link->cell)->mode;
				ForceChanMode(((irc_bot *)tmp_link->cell), ((chans *)chan_link->cell), 0);
				((chans *)chan_link->cell)->mode = 0;
				ForceChanMode(((irc_bot *)tmp_link->cell), ((chans *)chan_link->cell), i);
				((chans *)chan_link->cell)->mode = i;
			    }
			}
		    }
		}
	    }
	}
		
	FD_ZERO(&rd);
	FD_ZERO(&rw);
	timeout.tv_sec=1;
	timeout.tv_usec=0;
	for (tmp_link = bot_list; tmp_link; tmp_link=tmp_link->next)
	    if (((irc_bot *)tmp_link->cell)->state != STATE_NOT_CONNECTED)
		FD_SET(((irc_bot *)tmp_link->cell)->socknum,&rd);	
	select(max_fd, &rd, &rw, NULL, &timeout);
	for (tmp_link = bot_list; tmp_link; tmp_link=tmp_link->next) {
	    if (FD_ISSET(((irc_bot *)tmp_link->cell)->socknum, &rd)) {
		((irc_bot *)tmp_link->cell)->state *= ReadSocket(((irc_bot *)tmp_link->cell)->socknum,
							     ((irc_bot *)tmp_link->cell)->sockbuffer,
							     &((irc_bot *)tmp_link->cell)->sockbufferpos,
							     ((irc_bot *)tmp_link->cell)->readfromsocket);
		if (((irc_bot *)tmp_link->cell)->state == STATE_NOT_CONNECTED) {
		    ShortTime(the_time);
		    FreeList(&((irc_bot *)tmp_link->cell)->who_list);
		    LoopList(&((irc_bot *)tmp_link->cell)->host_list); /* force loop on socket error */
		    fprintf(((irc_bot *)tmp_link->cell)->outfile, "*** SOCKET ERROR [%s] on %s\n",
			    the_time, ((hosts *)((irc_bot *)tmp_link->cell)->host_list->cell)->name);
		} else {
		    for (parse_pos = strchr(((irc_bot *)tmp_link->cell)->readfromsocket,'\n'),
			    last_parsed = ((irc_bot *)tmp_link->cell)->readfromsocket;
			parse_pos ;
			parse_pos = strchr(parse_pos,'\n')) {
			if (*(parse_pos-1)=='\r') {
			    *(parse_pos-1)=0;
			    parse_pos++;
			} else
			    *parse_pos++ = 0;
			Parse(((irc_bot *)tmp_link->cell), last_parsed);
			last_parsed = parse_pos;
		    }
		    len = strlen(last_parsed);
		    if (*(last_parsed+len-1)=='\r')
			*(last_parsed+len-1)=0;
		    Parse(((irc_bot *)tmp_link->cell), last_parsed);
		}
	    } else {
		if (((irc_bot *)tmp_link->cell)->state == STATE_RUNNING)
		    if (difftime(time((time_t *)NULL), ((irc_bot *)tmp_link->cell)->s_ping)>((irc_bot *)tmp_link->cell)->max_ping) {
			((irc_bot *)tmp_link->cell)->state = STATE_NOT_CONNECTED;
			FreeList(&((irc_bot *)tmp_link->cell)->who_list);
			LoopList(&((irc_bot *)tmp_link->cell)->host_list); /* force loop on ping timeout */
			shutdown(((irc_bot *)tmp_link->cell)->socknum,2);
			close(((irc_bot *)tmp_link->cell)->socknum);
			ShortTime(the_time);
			fprintf(((irc_bot *)tmp_link->cell)->outfile, "*** PING TIMEOUT [%s] on %s\n",
				the_time, ((hosts *)((irc_bot *)tmp_link->cell)->host_list->cell)->name);
		    }
	    }
	}
	is_first = TRUE;
	for (tmp_link = bot_list; tmp_link;) {
	    if (((irc_bot *)tmp_link->cell)->quit) {
		SwallowLink(tmp_link);
		next_link = tmp_link->next;
		if (is_first && !tmp_link->next)
		    bot_list = NULL;
		else
		    if (is_first)
			bot_list = next_link;
		ShutBot((irc_bot *)tmp_link->cell);
		free(tmp_link);
		tmp_link = next_link;
	    } else {
		is_first = FALSE;
		tmp_link = tmp_link->next;
	    }
	}
    }
    return 0;
}
