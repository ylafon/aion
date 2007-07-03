/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: file.c,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include "defs.h"
#include "types.h"
#include "file.h"
#include "list.h"
#include "cell.h"
#include "compare.h"

void LoadAll(bot)
    irc_bot *bot;
{
    FILE *infile;
    char buffer[256];
    char tmp1[256],tmp2[256],*sdump;
    int dump, dump2, dump3;
    char cdump;
    int do_chan;
    int nick_nb = 0;
    int rand_nick;

    if ((infile=fopen(bot->configname,"r"))==NULL) {
	printf("Error while opening %s\n",bot->configname);
	exit(0);
    }
    FreeList(&bot->us_list);
    FreeList(&bot->host_list);
    do_chan = (bot->chan_list!=NULL);
    FreeList(&bot->chan_list); /* take care of freeing this... */
    *buffer=0;

    for (;strcmp(buffer, "&end");) {
	fscanf(infile,"%s",buffer);
	if (!strcmp(buffer, "&server")) {
	    fscanf(infile,"%s %d",tmp1,&dump);
	    AddCellLast(&bot->host_list, CELL_HOSTS, CreateHost(tmp1, dump));
	} else if (!strcmp(buffer, "&serverpass")) {
	    fscanf(infile,"%s %d %s", tmp1, &dump, tmp2);
	    AddCellLast(&bot->host_list, CELL_HOSTS, 
			CreateHostPass(tmp1, dump, tmp2));
	} else if (!strcmp(buffer, "&command")) {
	    fscanf(infile, "%s", tmp1); /* to skip the blanks */
	    bot->cmd_char = *tmp1;
	} else if (!strcmp(buffer, "&username")) {
	    fscanf(infile, "%s", tmp1);
	    if (bot->username)
		free(bot->username);
	    bot->username = (char *)malloc(strlen(tmp1)+1);
	    strcpy(bot->username, tmp1);
	} else if (!strcmp(buffer, "&realname")) {
	    sdump = tmp1;
	    for (fscanf(infile, "%c", &cdump); cdump==' '; fscanf(infile, "%c", &cdump));
		
	    for (;cdump!='\n'; fscanf(infile, "%c", &cdump))
		*sdump++ = cdump;
	    *sdump=0;
	    if (bot->realname)
		free(bot->realname);
	    bot->realname = (char *)malloc(strlen(tmp1)+1);
	    strcpy(bot->realname, tmp1);
	} else if (!strcmp(buffer, "&nick")) {
	    fscanf(infile,"%s",tmp1);
	    nick_nb++;
	    AddCellLast(&bot->nick_list, CELL_NICKS, CreateNick(tmp1));
	} else if (!strcmp(buffer, "&behaviour")) {
	    fscanf(infile, "%d", &dump);
	    bot->behaviour = dump;
	} else if (!strcmp(buffer, "&chan")) {
	    fscanf(infile, "%s %d %d", tmp1, &dump, &dump2);
	    dump  &= ~LOCK_TOPIC;
	    dump2 &= ~LOCK_TOPIC;
	    if (dump2 & PLUS_L)
		fscanf(infile, "%d", &dump3);
	    if (dump2 & PLUS_K)
		fscanf(infile, "%s", buffer);
	    
	    AddCellLast(&bot->chan_list, CELL_CHANS, CreateChannel(tmp1, dump, (dump2 & PLUS_K) ? buffer : NULL, (dump2 & PLUS_L) ? dump3 : 0, dump2));
	} else if (!strcmp(buffer, "&masterlist")) {
	    fscanf(infile, "%s %s", tmp1, tmp2);
	    UpdateUser(&bot->us_list, tmp1, tmp2, US_FRIEND, ETERNITY, FindUserStrict);
	} else if (!strcmp(buffer, "&userlist")) {
	    fscanf(infile,"%s %s",tmp1,tmp2);
	    UpdateUser(&bot->us_list, tmp1, tmp2, US_USER, ETERNITY, FindUserStrict);
	} else if (!strcmp(buffer, "&botlist")) {
	    fscanf(infile, "%s %s", tmp1, tmp2);
	    UpdateUser(&bot->us_list, tmp1, tmp2, US_BOT, ETERNITY, FindUserStrict);
	} else if (!strcmp(buffer, "&autooplist")) {
	    fscanf(infile, "%s %s", tmp1, tmp2);
	    UpdateUser(&bot->us_list, tmp1, tmp2, US_AUTOOP, ETERNITY, FindUserStrict);
	} else if (!strcmp(buffer, "&bprolist")) {
	    fscanf(infile,"%s %s",tmp1,tmp2);
	    UpdateUser(&bot->us_list, tmp1, tmp2, US_BPRO, ETERNITY, FindUserStrict);
	} else if (!strcmp(buffer, "&kprolist")) {
	    fscanf(infile, "%s %s", tmp1, tmp2);
	    UpdateUser(&bot->us_list, tmp1, tmp2, US_KPRO, ETERNITY, FindUserStrict);
	} else if (!strcmp(buffer, "&oprolist")) {
	    fscanf(infile, "%s %s", tmp1, tmp2);
	    UpdateUser(&bot->us_list, tmp1, tmp2, US_OPRO, ETERNITY, FindUserStrict);
	} else if (!strcmp(buffer,"&shitlist")) {
	    fscanf(infile,"%s %s",tmp1,tmp2);
	    UpdateUser(&bot->us_list, tmp1, tmp2, US_SHIT, ETERNITY, FindUserStrict);
	} else if (!strcmp(buffer, "&config_flag")) {
	    fscanf(infile, "%d", &bot->config_flag);
	}
    }
    
    if (bot->config_flag & FLAG_RANDOM_NICK_FIRST 
	&& bot->state != STATE_RUNNING) {
	rand_nick = rand() % nick_nb;
	for (dump = 0; dump < rand_nick; dump++)
	    LoopList(&bot->nick_list);
	printf("Picked first nick : %d on %d\n", rand_nick+1, nick_nb);
    }
    
    if (bot->config_flag & FLAG_ADD_COMMAND_CHAR
       && bot->state != STATE_RUNNING) {
	sdump = bot->realname;
	bot->realname = (char *)malloc(strlen(sdump)+5);
	sprintf(bot->realname, "%s (%c)", sdump, bot->cmd_char);
	free(sdump);
    }
    
    fclose(infile);
}

void SaveAll(bot)
    irc_bot *bot;
{
    FILE *outfile;
    list *dummylist;
    int dump;
    char *sdump;
    
    if (!bot->configname)
	return;
    
    if ((outfile=fopen(bot->configname,"w"))==NULL) {
	printf("Error while opening configname\n");
    } else {
	fprintf(outfile,"#Behaviour mask\n&behaviour %d\n", bot->behaviour);
	
	fprintf(outfile,"#Server_list\n");
	for (dummylist = bot->host_list; dummylist; dummylist = dummylist->next) {
	    if (((hosts *)dummylist->cell)->passwd) {
		fprintf(outfile,"&serverpass %s %d %s\n",
			((hosts *)dummylist->cell)->name,
			((hosts *)dummylist->cell)->port,
			((hosts *)dummylist->cell)->passwd);
	    } else {
		fprintf(outfile,"&server %s %d\n",
			((hosts *)dummylist->cell)->name, ((hosts *)dummylist->cell)->port);
	    }
	}
	
	fprintf(outfile,"#username\n&username %s\n",bot->username);
	
	if (bot->config_flag & FLAG_ADD_COMMAND_CHAR) {
	    dump = strlen(bot->realname);
	    sdump = (char *)malloc((unsigned int)dump-3);
	    strncpy(sdump, bot->realname, (unsigned int)dump-4);
	    sdump[dump-3] = 0;
	    fprintf(outfile, "#realname\n&realname %s\n", sdump);
	    free(sdump);
	} else
	    fprintf(outfile, "#realname\n&realname %s\n", bot->realname);
	
	if (bot->cmd_char != BOT_COMMAND_CHAR)
	    fprintf(outfile, "#command char\n&command %c\n", bot->cmd_char);
	if (bot->config_flag)
	    fprintf(outfile, "#config flags\n&config_flag %d\n",
		    bot->config_flag);

	fprintf(outfile,"#nick_list\n");
	for (dummylist = bot->nick_list; dummylist; dummylist = dummylist->next)
	    fprintf(outfile,"&nick %s\n", ((nicks *)dummylist->cell)->nick);

	fprintf(outfile,"#chan_list\n");
	for (dummylist = bot->chan_list; dummylist; dummylist = dummylist->next) {
	    fprintf(outfile,"&chan %s %d %d", ((chans *)dummylist->cell)->name,
		    ((chans *)dummylist->cell)->flags, ((chans *)dummylist->cell)->mode);
	    if (((chans *)dummylist->cell)->mode & PLUS_L)
		fprintf(outfile," %d", ((chans *)dummylist->cell)->size);
	    if (((chans *)dummylist->cell)->mode & PLUS_K)
		fprintf(outfile," %s", ((chans *)dummylist->cell)->passwd);
	    fprintf(outfile, "\n");
	}

	fprintf(outfile,"#master_list\n");
	for (dummylist = bot->us_list; dummylist; dummylist = dummylist->next)
	    if (((users *)dummylist->cell)->flags & US_FRIEND)
		if (((users *)dummylist->cell)->fr_time > (ETERNITY / 10))
		    fprintf(outfile,"&masterlist %s %s\n",
			    ((users *)dummylist->cell)->addr, ((users *)dummylist->cell)->channel);
	
	fprintf(outfile,"#user_list\n");
	for (dummylist = bot->us_list; dummylist; dummylist = dummylist->next)
	    if (((users *)dummylist->cell)->flags & US_USER)
	    if (((users *)dummylist->cell)->us_time > (ETERNITY / 10))
		fprintf(outfile,"&userlist %s %s\n",
			((users *)dummylist->cell)->addr, ((users *)dummylist->cell)->channel);

	fprintf(outfile,"#bot_list\n");
	for (dummylist = bot->us_list; dummylist; dummylist = dummylist->next)
	    if (((users *)dummylist->cell)->flags & US_BOT)
		if (((users *)dummylist->cell)->bot_time > (ETERNITY / 10))
		    fprintf(outfile,"&botlist %s %s\n",
			    ((users *)dummylist->cell)->addr, ((users *)dummylist->cell)->channel);

	fprintf(outfile,"#autoop_list\n");
	for (dummylist = bot->us_list; dummylist; dummylist = dummylist->next)
	    if (((users *)dummylist->cell)->flags & US_AUTOOP)
		if (((users *)dummylist->cell)->op_time > (ETERNITY / 10))
		    fprintf(outfile,"&autooplist %s %s\n",
			    ((users *)dummylist->cell)->addr, ((users *)dummylist->cell)->channel);

	fprintf(outfile,"#banpro_list\n");
	for (dummylist = bot->us_list; dummylist; dummylist = dummylist->next)
	    if (((users *)dummylist->cell)->flags & US_BPRO)
		if (((users *)dummylist->cell)->bpro_time > (ETERNITY / 10))
		    fprintf(outfile,"&bprolist %s %s\n",
			    ((users *)dummylist->cell)->addr, ((users *)dummylist->cell)->channel);

	fprintf(outfile,"#kickpro_list\n");
	for (dummylist = bot->us_list; dummylist; dummylist = dummylist->next)
	    if (((users *)dummylist->cell)->flags & US_KPRO)
		if (((users *)dummylist->cell)->kpro_time > (ETERNITY / 10))
		    fprintf(outfile,"&kprolist %s %s\n",
			    ((users *)dummylist->cell)->addr, ((users *)dummylist->cell)->channel);
	
	fprintf(outfile,"#oppro_list\n");
	for (dummylist = bot->us_list; dummylist; dummylist = dummylist->next)
	    if (((users *)dummylist->cell)->flags & US_OPRO)
		if (((users *)dummylist->cell)->opro_time > (ETERNITY / 10))
		    fprintf(outfile,"&oprolist %s %s\n",
			    ((users *)dummylist->cell)->addr, ((users *)dummylist->cell)->channel);
	
	fprintf(outfile,"#shit_list\n");
	for (dummylist = bot->us_list; dummylist; dummylist = dummylist->next)
	    if (((users *)dummylist->cell)->flags & US_SHIT)
		if (((users *)dummylist->cell)->sh_time > (ETERNITY / 10))
		    fprintf(outfile,"&shitlist %s %s\n",
			    ((users *)dummylist->cell)->addr, ((users *)dummylist->cell)->channel);
	
	fprintf(outfile,"&end\n");
	fclose(outfile);
    }
}
