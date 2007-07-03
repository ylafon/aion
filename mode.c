/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include "defs.h"
#include "types.h"
#include "mode.h"
#include "compare.h"
#include "list.h"
#include "cell.h"
#include "action.h"
#include "match.h"
#include "time.h"
#include "strip.h"

void ModeAnalysis(bot, channel, raw_mode, name, addr)
    irc_bot *bot;
    char *channel, *raw_mode, *name, *addr;
{
    chans *the_channel,*dummychan;
    users *the_user,*dummyuser,*the_bot;
    who   *the_who,*dummywho;
    hacks *dummyhack;
    list  *the_hack, *dummylist, **dummylink;
    todos *dummytodo;
    char  *mod, *mod_obj;
    char  *copyc;
    char  objs[16][128];
    char  m_objs[16][3];
    char  todo_objs[512];
    char  *todo_objs_tmp;
    char  the_time[64];
    char  sdump[128];
    char  p_m='+';
    int   i, nbmods;
    int   todo;
    int   dorevenge;
    int   act_of_god;
    int   fromserver;
    int   do_k;
    int   do_ob;

    todo = FALSE;
    do_k = FALSE;
    do_ob = FALSE;
    dorevenge = FALSE;
    todo_objs_tmp = todo_objs;
    fromserver = (strchr(name, '.') != 0);

    dummychan = CreateChannel(channel, 0,(char *)NULL, 0, 0);
    dummylist = FindCell(&bot->chan_list, dummychan, FindChannel);
    FreeCell(CELL_CHANS, dummychan);
    if (dummylist)
	the_channel = dummylist->cell;
    else
	return; /* usermode change */
    dummyuser = CreateUser(addr, channel, US_MASTER);
    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
    FreeCell(CELL_USERS, dummyuser);
    act_of_god = (dummylist!=NULL) || !strcmp(name,((nicks *)bot->nick_list->cell)->nick);
   
    dummyuser = CreateUser(addr, channel, US_BOT);
    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
    FreeCell(CELL_USERS, dummyuser);
    the_bot = (users *)((dummylist) ? ((users *)dummylist->cell) : NULL);
    mod = raw_mode;
    while (*mod==' '&&*mod) /* probably unnecessary */
	mod++;

    if (*mod==':')
        mod++;

    mod_obj = mod;
    while (*mod_obj!=' '&&*mod_obj)
	mod_obj++;

    if (*mod_obj)
        *mod_obj++=0; /* add end of line */
    
    nbmods=0;
    while (*mod) {
	switch (*mod) {
	case '+':
	    p_m = '+';
	    break;
	case '-':
	    p_m = '-';
	    break;
	case 'l':
	    if (p_m=='-') {
		sprintf(m_objs[nbmods],"%c%c",p_m,*mod);
		nbmods++;
		break;
	    }
	case 'o':
	case 'b':
	case 'k':
	case 'v':
	    copyc = objs[nbmods];
	    while (*mod_obj!=' '&&*mod_obj)
		*copyc++ = *mod_obj++;
	    *copyc=0;
	    mod_obj++;
	case 'i':
	case 'p':
	case 's':
	case 'n':
	case 'm':
	case 't':
	    sprintf(m_objs[nbmods],"%c%c",p_m,*mod);
	    nbmods++;
	    break;
	default:
	    printf("**** PANIC! **** UNKNOWN MODE CHANGE [%c] in ModeAnalysis\n", *mod);
	    break;
	};
	mod++;
    };
    
    for (i=0;i<nbmods;i++) {
	if (m_objs[i][1]=='l') {
	    if (!the_bot && (the_channel->flags & KEEP_L)) {
		if (*m_objs[i]=='+') {
		    if (the_channel->mode & PLUS_L) {
			if (the_channel->size != atoi(objs[i])) {
			    sprintf(todo_objs_tmp," +l %d",the_channel->size);
			    todo_objs_tmp+=strlen(todo_objs_tmp);
			    todo++;
			    dorevenge++;
			}
		    } else {
			strcpy(todo_objs_tmp," -l");
			todo_objs_tmp+=strlen(todo_objs_tmp);
			todo++;
			dorevenge++;
		    }
		} else if (the_channel->mode & PLUS_L) {
		    sprintf(todo_objs_tmp," +l %d",the_channel->size);
		    todo_objs_tmp+=strlen(todo_objs_tmp);
		    todo++;
		    dorevenge++;
		}
	    }

	    if (*m_objs[i]=='+') {
		the_channel->size = atoi(objs[i]);
		the_channel->mode |= PLUS_L;
	    } else
		the_channel->mode &= ~PLUS_L; 
	} else if (m_objs[i][1]=='k') {
	    if (!the_bot && (the_channel->flags & KEEP_K)) {
		if (*m_objs[i]=='+') {
		    if (!(the_channel->mode & PLUS_K)) {
			sprintf(todo_objs_tmp," -k %s",objs[i]);
			todo_objs_tmp+=strlen(todo_objs_tmp);
			todo++;
			do_k++;
			dorevenge++;
		    } else { /* +k over a +k... that IS nasty! */
			printf("mode +k %s sur +k %s on %s\n", objs[i],the_channel->passwd, channel);
			if (strcmp(objs[i], the_channel->passwd)) {
			    sprintf(todo_objs_tmp," -k %s",objs[i]);
			    todo_objs_tmp += strlen(todo_objs_tmp);
			    todo++;
			    do_k++;      /* +k is delayed to prevent interfering with -k-o */
			    dorevenge++;
			    sprintf(todo_objs_tmp, "MODe %s +k %s\n", channel, the_channel->passwd);
			    AddCellFirst(&bot->todo_list,CELL_TODOS,
					 CreateTodo(todo_objs_tmp, MODE_K_TIME));
			    *todo_objs_tmp = 0;
			}
		    }
		} else if (the_channel->mode & PLUS_K) {
		    sprintf(todo_objs_tmp," +k %s",objs[i]);
		    todo_objs_tmp+=strlen(todo_objs_tmp);
		    todo++;
		    do_k++;
		    dorevenge++;
		}
	    }
	    if (*m_objs[i]=='+') {
		the_channel->mode |= PLUS_K;
		if (the_channel->passwd)
		    free(the_channel->passwd);
		the_channel->passwd = (char *)malloc(strlen(objs[i])+1);
		strcpy(the_channel->passwd,objs[i]);
	    }
	    else
		the_channel->mode &= ~PLUS_K;
	} else if (m_objs[i][1]=='i') {
	    if (!the_bot && (the_channel->flags & KEEP_I)) {
		if (*m_objs[i]=='+') {
		    if (!(the_channel->mode & PLUS_I)) {
			strcpy(todo_objs_tmp," -i");
			todo_objs_tmp+=strlen(todo_objs_tmp);
			todo++;
			dorevenge++;
		    }
		} else if (the_channel->mode & PLUS_I) {
		    strcpy(todo_objs_tmp," +i");
		    todo_objs_tmp+=strlen(todo_objs_tmp);
		    todo++;
		    dorevenge++;
		}
	    }
	    if (*m_objs[i]=='+')
		the_channel->mode |= PLUS_I;
	    else
		the_channel->mode &= ~PLUS_I;
	} else if (m_objs[i][1]=='p') {
	    if (!the_bot && (the_channel->flags & KEEP_P)) {
		if (*m_objs[i]=='+') {
		    if (!(the_channel->mode & PLUS_P)) {
			strcpy(todo_objs_tmp," -p");
			todo_objs_tmp+=strlen(todo_objs_tmp);
			todo++;
			dorevenge++;
		    }
		} else if (the_channel->mode & PLUS_P) {
		    strcpy(todo_objs_tmp," +p");
		    todo_objs_tmp+=strlen(todo_objs_tmp);
		    todo++;
		    dorevenge++;
		}
	    }
	    if (*m_objs[i]=='+')
		the_channel->mode |= PLUS_P;
	    else
		the_channel->mode &= ~PLUS_P;
	} else if (m_objs[i][1]=='s') {
	    if (!the_bot && (the_channel->flags & KEEP_S)) {
		if (*m_objs[i]=='+') {
		    if (!(the_channel->mode & PLUS_S)) {
			strcpy(todo_objs_tmp," -s");
			todo_objs_tmp+=strlen(todo_objs_tmp);
			todo++;
			dorevenge++;
		    }
		} else if (the_channel->mode & PLUS_S) {
		    strcpy(todo_objs_tmp," +s");
		    todo_objs_tmp+=strlen(todo_objs_tmp);
		    todo++;
		    dorevenge++;
		}
	    }
	    if (*m_objs[i]=='+')
		the_channel->mode |= PLUS_S;
	    else
		the_channel->mode &= ~PLUS_S;
	} else if (m_objs[i][1]=='n') {
	    if (!the_bot && (the_channel->flags & KEEP_N)) {
		if (*m_objs[i]=='+') {
		    if (!(the_channel->mode & PLUS_N)) {
			strcpy(todo_objs_tmp," -n");
			todo_objs_tmp+=strlen(todo_objs_tmp);
			todo++;
		    }
		} else if (the_channel->mode & PLUS_N) {
		    strcpy(todo_objs_tmp," +n");
		    todo_objs_tmp+=strlen(todo_objs_tmp);
		    todo++;
		}
	    }
	    if (*m_objs[i]=='+')
		the_channel->mode |= PLUS_N;
	    else
		the_channel->mode &= ~PLUS_N;
	} else if (m_objs[i][1]=='m') {
	    if (!the_bot && (the_channel->flags & KEEP_M)) {
		if (*m_objs[i]=='+') {
		    if (!(the_channel->mode & PLUS_M)) {
			strcpy(todo_objs_tmp," -m");
			todo_objs_tmp+=strlen(todo_objs_tmp);
			todo++;
			dorevenge++;
		    }
		} else if (the_channel->mode & PLUS_M) {
		    strcpy(todo_objs_tmp," +m");
		    todo_objs_tmp+=strlen(todo_objs_tmp);
		    todo++;
		    dorevenge++;
		}
	    }
	    if (*m_objs[i]=='+')
		the_channel->mode |= PLUS_M;
	    else
		the_channel->mode &= ~PLUS_M;
	} else if (m_objs[i][1]=='t') {
	    if (!the_bot && (the_channel->flags & KEEP_T)) {
		if (*m_objs[i]=='+') {
		    if (!(the_channel->mode & PLUS_T)) {
			strcpy(todo_objs_tmp," -t");
			todo_objs_tmp+=strlen(todo_objs_tmp);
			todo++;
		    }
		} else if (the_channel->mode & PLUS_T) {
		    strcpy(todo_objs_tmp," +t");
		    todo_objs_tmp+=strlen(todo_objs_tmp);
		    todo++;
		}
	    }
	    if (*m_objs[i]=='+')
		the_channel->mode |= PLUS_T;
	    else
		the_channel->mode &= ~PLUS_T;
	} else if (!strcmp(m_objs[i],"+b")) {
	    /* hard things begin here */
	    if (!(the_channel->flags & KEEP_B)) {
		dummyuser = CreateUser(strchr(objs[i], '!')+1, channel, US_BPRO);
		dummylist = FindCell(&bot->us_list, dummyuser, FindBanUser);
		FreeCell(CELL_USERS, dummyuser);
	    }
	    if (dummylist || (the_channel->flags & KEEP_B)) {
		sprintf(todo_objs_tmp," -b %s",objs[i]);
		todo_objs_tmp+=strlen(todo_objs_tmp);
		todo++;
		do_ob++;
		dorevenge++;  
	    }
	} else if (!strcmp(m_objs[i],"-b")) {
	    if (!(the_channel->flags & KEEP_B)) {
		dummyuser = CreateUser(strchr(objs[i], '!')+1, channel, US_SHIT|US_REBAN);
		dummylist = FindCell(&bot->us_list, dummyuser, FindBanUser);
		FreeCell(CELL_USERS, dummyuser);
	    }
	    if (dummylist || (the_channel->flags & KEEP_B)) {
		dummyuser = CreateUser(strchr(objs[i], '!')+1, channel, US_BPRO);
		dummylist = FindCell(&bot->us_list, dummyuser, FindBanUser);
		FreeCell(CELL_USERS, dummyuser);
		if (!dummylist) {
		    sprintf(todo_objs_tmp," +b %s",objs[i]);
		    todo_objs_tmp+=strlen(todo_objs_tmp);
		    todo++;
		    do_ob++;
		    dorevenge++; /* is this necessary ? */ 
		}
	    }
	    sprintf(sdump, "mOdE %s -b %s\n", channel, objs[i]);
	    dummytodo = CreateTodo(sdump, 0);
	    dummylink = FindCellAddr(&bot->todo_list, dummytodo, FindTodo);
	    FreeLink(dummylink);
	    FreeCell(CELL_TODOS, dummytodo);
	} else if (!strcmp(m_objs[i],"-o")) {
	    if (Match(objs[i],((nicks *)bot->nick_list->cell)->nick))
		the_channel->iamchanop=FALSE;
	    dummywho = CreateWho(objs[i], NULL, channel);
	    dummylist = FindCell(&bot->who_list, dummywho, FindWhoChan);
	    the_who = (dummylist) ? dummylist->cell : NULL;
	    FreeCell(CELL_WHO, dummywho);
	    if (the_who) {
		if (the_who->chanop) { /* doesn't matter if we deop a non-op */
		    dummyuser = CreateUser(the_who->addr, channel, US_OPRO);
		    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
		    FreeCell(CELL_USERS, dummyuser);
		    if (dummylist || (the_channel->flags & KEEP_O)) {
			sprintf(todo_objs_tmp," +o %s",objs[i]);
			todo_objs_tmp+=strlen(todo_objs_tmp);
			todo++;
			do_ob++;
			dorevenge++;
		    }
		}
		the_who->chanop=FALSE;
	    }
	} else if (!strcmp(m_objs[i],"+o")) {
	    if (Match(objs[i],((nicks *)bot->nick_list->cell)->nick)) {
		ShortTime(the_time);
		the_channel->iamchanop=TRUE;
		fprintf(bot->outfile,"**** OPED %s BY %s!%s on %s\n",
			       the_time, name, addr, channel);
	    }
	    dummywho = CreateWho(objs[i], NULL, channel);
	    dummylist = FindCell(&bot->who_list, dummywho, FindWhoChan);
	    FreeCell(CELL_WHO, dummywho);
	    if (dummylist) {
		the_who = dummylist->cell;
		if (fromserver && (the_channel->flags & NO_HACK)) {
		    dummyuser = CreateUser(the_who->addr, channel, US_MASTER|US_FRIEND|US_USER|US_OPRO|US_AUTOOP);
		    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
		    the_user = (users *)((dummylist) ? dummylist->cell : NULL);  
		    FreeCell(CELL_USERS, dummyuser);
		    dummyhack = CreateHack(channel, objs[i], the_who->addr);
		    dummylink = FindCellAddr(&bot->hack_list, dummyhack, FindHack);
		    the_hack = (dummylink) ? *dummylink : NULL;
		    FreeCell(CELL_HACKS, dummyhack);
		    if (the_hack)  { /* people in split oped by server -> removing from todo&hack lists */
			FreeLink(dummylink);
			sprintf(sdump, "MoDe %s +o %s\n", channel, objs[i]);
			dummytodo = CreateTodo(sdump, 0);
			dummylink = FindCellAddr(&bot->todo_list, dummytodo, FindTodo);
			FreeLink(dummylink);
			FreeCell(CELL_TODOS, dummytodo);
		    } else if (!the_user) {
			sprintf(todo_objs_tmp," -o %s",objs[i]);
			todo_objs_tmp+=strlen(todo_objs_tmp);
			todo++;
			do_ob++;
		    }
		} else {   /* remove from slowop but NOT from hacklist */
		    sprintf(sdump, "MoDe %s +o %s\n", channel, objs[i]);
		    dummytodo = CreateTodo(sdump, 0);
		    dummylink = FindCellAddr(&bot->todo_list, dummytodo, FindTodo);
		    FreeLink(dummylink); 
		    FreeCell(CELL_TODOS, dummytodo);
		    if (!the_who->chanop) {
			if (the_channel->flags & KEEP_O) {
			    dummyuser = CreateUser(the_who->addr, channel, US_MASTER|US_OPRO);
			    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
			    FreeCell(CELL_USERS, dummyuser);
			    if (!dummylist) {
				sprintf(todo_objs_tmp," -o %s",objs[i]);
				todo_objs_tmp+=strlen(todo_objs_tmp);
				todo++;
				do_ob++;
				dorevenge++;
			    }
			} else {
			    dummyuser = CreateUser(the_who->addr, channel, US_REBAN);
			    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
			    FreeCell(CELL_USERS, dummyuser);
			    if (dummylist) {
				sprintf(todo_objs_tmp," -o %s",objs[i]);
				todo_objs_tmp+=strlen(todo_objs_tmp);
				todo++;
				do_ob++;
				dorevenge++;
			    }
			}
		    }
		}
		if (the_who)
		    the_who->chanop = TRUE;
	    }
	}
	/* 
	 *  we don't check mode +v, it is useless here ;) 
	 *  Maybe in Aion v6, if i want to keep a channel silent...
	 */
    }
    if (dorevenge && (the_channel->flags & REVENGE)) {
	if (!fromserver) { /* no revenge if server-mode */
     	    dummyuser = CreateUser(addr, channel, US_BOT|US_MASTER|US_FRIEND|US_USER|US_OPRO);
	    dummylist = FindCell(&bot->us_list, dummyuser, FindUser);
	    FreeCell(CELL_USERS, dummyuser);
	    if (!dummylist) {
		copyc = (char *)malloc(strlen(addr)+2);
		if (IsNumeric(addr))
		    StripNumeric(addr,copyc);
		else
		    Strip(addr, copyc);
		UpdateUser(&bot->us_list, copyc, channel, US_REBAN, bot->revenge_time, FindUserStrict);
		free(copyc);
		if (!act_of_god) {
		    if (todo<3)
			sprintf(todo_objs_tmp," -o %s",name);
		    else
			Kick(bot, channel, name, "Behave yourself!");
		}
	    }
	}
    }
    if (todo) {
	ShortTime(the_time);
	if (act_of_god) {
	    if (strcmp(name,((nicks *)bot->nick_list->cell)->nick))
		fprintf(bot->outfile,"**** ACT OF GOD %s FROM %s!%s on %s -> [%s]\n",
			the_time, name, addr, channel, raw_mode);
	} else {
	    if (the_channel->iamchanop) {
		the_channel->last_mode = time((time_t *)NULL);
		if (bot->verbose)
		    printf("fromserver? %s, [%s]\n", (fromserver ? "Yes" : "No"), todo_objs+1);
		if (fromserver && do_ob && do_k) {
		    if (bot->verbose)
			printf("cutting [%s] ", todo_objs+1);
		    todo_objs_tmp = todo_objs;
		    while ((*(todo_objs_tmp+1)!='k' || (*todo_objs_tmp != '+' && *todo_objs_tmp!= '-')) &&
			   (*(todo_objs_tmp+1)!='o' || (*todo_objs_tmp != '+' && *todo_objs_tmp!= '-')))
			todo_objs_tmp++; /* we know that there IS a +/- k or o */
		    while (*todo_objs_tmp++ != ' '); /* skip mode */
		    while (*todo_objs_tmp && *todo_objs_tmp != ' ')
			todo_objs_tmp++; /* then the object */
		    mod = todo_objs_tmp;
		    *(mod++)=0;
		    Mode(bot, channel, "", todo_objs+1);
		    Mode(bot, channel, "", mod);
		    if (bot->verbose)
			printf("-> [%s] and [%s]\n", todo_objs+1, mod);
		    fprintf(bot->outfile,"**** NASTY ACTION %s FROM SERVER %s on %s -> [%s] & [%s]\n",
				the_time, name, channel, todo_objs+1, mod);
		} else {
		    Mode(bot, channel, "", todo_objs+1);
		    if (fromserver)
			fprintf(bot->outfile,"**** BAD ACTION %s FROM SERVER %s on %s -> [%s]\n",
				the_time, name, channel, todo_objs+1);
		    else
			fprintf(bot->outfile,"**** BAD ACTION %s FROM %s!%s on %s -> [%s]\n",
				the_time, name, addr, channel, todo_objs+1);
		}
	    } else {
		if (fromserver)
		    fprintf(bot->outfile,"**** UNDONE ACTION %s FROM SERVER %s on %s -> [%s]\n",
			    the_time, name, channel, todo_objs+1);
		else
		    fprintf(bot->outfile,"**** UNDONE ACTION %s FROM %s!%s on %s -> [%s]\n",
			    the_time, name, addr, channel, todo_objs+1);
	    }
	}
    }
}

void ForceChanMode(bot, channel, wanted_mode)
    irc_bot *bot;
    chans *channel;
    int wanted_mode;
{
    char todo[512];
    char the_time[64];
    char *todo_tmp;
    int diff_mode;
    int do_it;

    do_it = FALSE;
    todo_tmp = todo;
    if (!channel->iamchanop) {
	sprintf(todo_tmp, "MOde %s ", channel->name);
	todo_tmp += strlen(todo_tmp);
    }
    diff_mode = channel->mode ^ wanted_mode;
    if ((diff_mode & PLUS_L) && (channel->flags & KEEP_L)) {
	if (wanted_mode & PLUS_L)
	    sprintf(todo_tmp, "+l %d ", channel->size);
    	else
	    strcpy(todo_tmp, "-l ");
	todo_tmp += strlen(todo_tmp);
	do_it++;
    }
    
    if ((diff_mode & PLUS_K) && (channel->flags & KEEP_K)) {
	sprintf(todo_tmp, (wanted_mode & PLUS_K) ? "+k %s " : "-k %s ", channel->passwd);
	todo_tmp += strlen(todo_tmp);
	do_it++;
    }

    if ((diff_mode & PLUS_I) && (channel->flags & KEEP_I)) {
	strcpy(todo_tmp, (wanted_mode & PLUS_I) ? "+i " : "-i ");
	todo_tmp += strlen(todo_tmp);
	do_it++;
    }

    if ((diff_mode & PLUS_P) && (channel->flags & KEEP_P)) {
	strcpy(todo_tmp, (wanted_mode & PLUS_P) ? "+p " : "-p ");
	todo_tmp += strlen(todo_tmp);
	do_it++;
    }

    if ((diff_mode & PLUS_S) && (channel->flags & KEEP_S)) {
	strcpy(todo_tmp, (wanted_mode & PLUS_S) ? "+s " : "-s ");
	todo_tmp += strlen(todo_tmp);
	do_it++;
    }

    if ((diff_mode & PLUS_M) && (channel->flags & KEEP_M)) {
	strcpy(todo_tmp, (wanted_mode & PLUS_M) ? "+m " : "-m ");
	todo_tmp += strlen(todo_tmp);
	do_it++;
    }

    if ((diff_mode & PLUS_N) && (channel->flags & KEEP_N)) {
	strcpy(todo_tmp, (wanted_mode & PLUS_N) ? "+n " : "-n ");
	todo_tmp += strlen(todo_tmp);
	do_it++;
    }

    if ((diff_mode & PLUS_T) && (channel->flags & KEEP_T)) {
	strcpy(todo_tmp, (wanted_mode & PLUS_T) ? "+t " : "-t ");
	todo_tmp += strlen(todo_tmp);
	do_it++;
    }
    
    if (do_it) {
	*--todo_tmp = (!channel->iamchanop) * '\n';

	channel->last_mode = time((time_t *)NULL);
	if (channel->iamchanop)
	    Mode(bot, channel->name, "", todo);
	else
	    AddCellFirst(&bot->todo_list, CELL_TODOS, CreateTodo(todo, CHAN_MODE_ON_JOIN_DELAY));

	if (bot->verbose)
	    printf("%s [%s] on channel %s\n", (channel->iamchanop) ? "doing" : "delaying", todo, channel->name);
	ShortTime(the_time);
	fprintf(bot->outfile, "*** %s [%s] %s on channel %s\n", (channel->iamchanop)? "CORRECTING" : "DELAYING CORRECTION", the_time, todo, channel->name);
    }
}
    
void ChanModeAnalysis(bot, channel, raw_mode)
    irc_bot *bot;
    char *channel, *raw_mode;
{
    char  *mod, *mod_obj;
    char  *copyc;
    char  size_buffer[16];
    char  p_m='+';
    chans *the_channel,*dummychan;
    list  *dummylist;
    int  wanted_mode;

    dummychan = CreateChannel(channel, 0,(char *)NULL, 0, 0);
    dummylist = FindCell(&bot->chan_list, dummychan, FindChannel);
    if (dummylist)
	the_channel = dummylist->cell;
    else {
	printf("**** PANIC! **** UNKNOWN CHANNEL %s!\n",channel);
	return; /* error, channel not found */
    }
    FreeCell(CELL_CHANS, dummychan);
    wanted_mode = the_channel->mode;

    mod = raw_mode;
    while (*mod==' '&&*mod) /* probably unnecessary */
	mod++;
    mod_obj = mod;
    while (*mod_obj!=' '&&*mod_obj)
	mod_obj++;
    if (*mod_obj)
        *mod_obj++=0; /* add end of line */
    the_channel->mode = 0;
    while (*mod) {
	switch(*mod) {
	case '+':
	    p_m = '+';
	    break;
	case '-':
	    p_m = '-';
	    break;
	case 'l':
	    if (p_m=='-')
		the_channel->mode &= ~PLUS_L;
	    else {
		the_channel->mode |= PLUS_L;
		copyc = size_buffer;
		while (*mod_obj!=' '&&*mod_obj)
		    *copyc++ = *mod_obj++;
		*copyc=0;
		mod_obj++;
		the_channel->size = atoi(size_buffer);
	    }
	    break;
	case 'k':
	    if (p_m=='-')
		the_channel->mode &= ~PLUS_K;
	    else
		the_channel->mode |= PLUS_K;
	    the_channel->mode |= PLUS_K;
	    copyc = mod_obj;
	    while (*copyc!=' '&&*copyc)
		copyc++;
	    if (the_channel->passwd)
		free(the_channel->passwd);
	    the_channel->passwd = (char *)malloc((unsigned int)(copyc - mod_obj + 1));
	    copyc = the_channel->passwd;
	    while (*mod_obj!=' '&&*mod_obj)
		*copyc++ = *mod_obj++;
	    *copyc=0;
	    mod_obj++;
	    break;
	case 'i':
	    if (p_m=='-')
		the_channel->mode &= ~PLUS_I;
	    else
		the_channel->mode |= PLUS_I;
	    break;
	case 'p':
	    if (p_m=='-')
		the_channel->mode &= ~PLUS_P;
	    else
		the_channel->mode |= PLUS_P;
	    break;
	case 's':
	    if (p_m=='-')
		the_channel->mode &= ~PLUS_S;
	    else
		the_channel->mode |= PLUS_S;
	    break;
	case 'n':
	    if (p_m=='-')
		the_channel->mode &= ~PLUS_N;
	    else
		the_channel->mode |= PLUS_N;
	    break;
	case 'm':
	    if (p_m=='-')
		the_channel->mode &= ~PLUS_M;
	    else
		the_channel->mode |= PLUS_M;
	    break;
	case 't':
	    if (p_m=='-')
		the_channel->mode &= ~PLUS_T;
	    else
		the_channel->mode |= PLUS_T;
	    break;
	case 'c':
	    if (p_m=='-')
		the_channel->mode &= ~PLUS_C;
	    else
		the_channel->mode |= PLUS_C;
	    break;
	default:
		printf("**** PANIC! **** UNKNOWN MODE CHANGE [%c] in ChanModeAnalysis\n", *mod);
	    break;
	}
	mod++;
    }
    if (the_channel->mode != wanted_mode)
   	ForceChanMode(bot, the_channel, wanted_mode);
}
