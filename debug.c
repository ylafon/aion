/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: debug.c,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include "defs.h"
#include "types.h"
#include "debug.h"
#include "list.h"
#include "time.h"

void Debug(bot)
    irc_bot *bot;
{
    FILE *debug_file;
    list *tmplink;
    void *cell;

    debug_file = fopen("debug.html","w");
    if (!debug_file)
	return;
    fprintf(debug_file, "<HTML><HEAD><TITLE>Aion debug file</TITLE><STYLE>\n");
    fprintf(debug_file, "H1 { font-style: small-caps; color: red }\nHTML { background : #BFBFBF }\n");
    fprintf(debug_file, "</STYLE></HEAD><BODY>\n");
    fprintf(debug_file, "<H1>Infos G&eacute;n&eacute;rales</H1>\n");
    fprintf(debug_file, "<UL>\n");
    fprintf(debug_file, "<LI>Command    : %c\n", bot->cmd_char);
    fprintf(debug_file, "<LI>Quit       : %d\n", bot->quit);
    fprintf(debug_file, "<LI>Verbose    : %d\n", bot->verbose);
    fprintf(debug_file, "<LI>Behaviour  : %d\n", bot->behaviour); 
    fprintf(debug_file, "<LI>Logname    : %s\n", bot->logname);
    fprintf(debug_file, "<LI>Configname : %s\n", bot->configname);
    fprintf(debug_file, "<LI>ConfigFlag : %d\n", bot->config_flag);
    if (bot->cmd_char != '<')
      fprintf(debug_file, "<LI>Command    : %c\n", bot->cmd_char);
    else
      fprintf(debug_file, "<LI>Command    : &lt;\n");
    fprintf(debug_file, "<LI>Uptime     : %ds\n", (int)difftime(time((time_t *)NULL),bot->uptime));
    fprintf(debug_file, "<LI>Ping time  : %ds\n", (int)difftime(time((time_t *)NULL),bot->s_ping));
    fprintf(debug_file, "<LI>Username   : %s\n", bot->username);
    fprintf(debug_file, "<LI>Realname   : %s\n", bot->realname);
    fprintf(debug_file, "<LI>hack_time  : %ds\n", bot->hack_max_time);
    fprintf(debug_file, "<LI>max_ping   : %ds\n", bot->max_ping);
    fprintf(debug_file, "<LI>slow reop  : %ds\n", bot->hack_reop_time);
    fprintf(debug_file, "<LI>revenge    : %ds\n", bot->revenge_time);
    fprintf(debug_file, "<LI>lame fake  : %ds\n", bot->lame_fake_time);
    fprintf(debug_file, "<LI>loop nick  : %ds\n", bot->loop_nick_time);
    fprintf(debug_file, "<LI>war time   : %ds\n", bot->war_time);
    fprintf(debug_file, "<LI>Nb_notice  : %d\n", bot->nb_notice);
    fprintf(debug_file, "</UL>\n<H1>Liste des serveurs</H1>\n");
    fprintf(debug_file, "<TABLE BORDER><TR><TH>Name<TH>port<TH>Uptime</TR>\n");
    for (tmplink = bot->host_list; tmplink ; tmplink = tmplink->next) {
	cell = tmplink->cell;
	if (tmplink->type == CELL_HOSTS)
	    fprintf(debug_file,"<TR><TD>%s<TD>%d<TD>%ds</TR>\n",
		    ((hosts *)cell)->name, ((hosts *)cell)->port,
		    (int)difftime(time((time_t *)NULL),((hosts *)cell)->uptime));
	else
	    fprintf(debug_file,"<TR><TH COLSPAN=3>ERROR! CELL_TYPE %d</TR>\n", tmplink->type);
    }
    fprintf(debug_file, "<TR><TH>Name<TH>port<TH>Uptime</TR>\n</TABLE>\n");
    
    fprintf(debug_file, "<H1>Liste des nicks</H1>\n");
    fprintf(debug_file, "<TABLE BORDER><TR><TH>Nick<TH>Uptime</TR>\n");
    for (tmplink = bot->nick_list; tmplink ; tmplink = tmplink->next) {
	cell = tmplink->cell;
	if (tmplink->type == CELL_NICKS)
	    fprintf(debug_file,"<TR><TD>%s<TD>%ds</TR>\n", ((nicks *)cell)->nick,
		    (int)difftime(time((time_t *)NULL),((nicks *)cell)->uptime));
	else
	    fprintf(debug_file,"<TR><TH COLSPAN=2>ERROR! CELL_TYPE %d</TR>\n", tmplink->type);
    }
    fprintf(debug_file, "<TR><TH>Nick<TH>Uptime</TR>\n</TABLE>\n");
    
    fprintf(debug_file, "<H1>Liste des utilisateurs</H1>\n");
    fprintf(debug_file, "<TABLE BORDER><TR><TH>Addr<TH>Chan<TH>Uptime<TH>Flags");
    fprintf(debug_file, "<TH>Actions<TH>Bad Actions</TR>\n");
    for (tmplink = bot->us_list; tmplink ; tmplink = tmplink->next) {
	cell = tmplink->cell;
	if (tmplink->type == CELL_USERS)
	    fprintf(debug_file,"<TR><TD>%s<TD>%s<TD>%ds<TD>%d<TD>%d<TD>%d</TR>\n", 
		    ((users *)cell)->addr, ((users *)cell)->channel,
		    (int)difftime(time((time_t *)NULL),((users *)cell)->uptime),
		    ((users *)cell)->flags, ((users *)cell)->actions,
		    ((users *)cell)->bad_actions);
	else
	    fprintf(debug_file,"<TR><TH COLSPAN=6>ERROR! CELL_TYPE %d</TR>\n", tmplink->type);
    }
    fprintf(debug_file, "<TR><TH>Addr<TH>Chan<TH>Uptime<TH>Flags<TH>Actions");
    fprintf(debug_file, "<TH>Bad Actions</TR>\n</TABLE>\n");
    
    fprintf(debug_file, "<H1>Liste des personnes pr&eacute;sentes</H1>\n");
    fprintf(debug_file, "<TABLE BORDER><TR><TH>Nick<TH>Addr<TH>Chan<TH>Uptime<TH>ChanOp</TR>\n");
    for (tmplink = bot->who_list; tmplink ; tmplink = tmplink->next) {
	cell = tmplink->cell;
	if (tmplink->type == CELL_WHO)
	    fprintf(debug_file,"<TR><TD>%s<TD>%s<TD>%s<TD>%ds<TD>%s</TR>\n",
		    ((who *)cell)->nick, ((who *)cell)->addr, ((who *)cell)->chan,
		    (int)difftime(time((time_t *)NULL),((who *)cell)->uptime),
		    ((who *)cell)->chanop ? "Op" : "Not op");
	else
	    fprintf(debug_file,"<TR><TH COLSPAN=5>ERROR! CELL_TYPE %d</TR>\n", tmplink->type);
    }
    fprintf(debug_file, "<TR><TH>Nick<TH>Addr<TH>Chan<TH>Uptime<TH>ChanOp</TR>\n</TABLE>\n");
    
    fprintf(debug_file, "<H1>Liste des chans</H1>\n");
    fprintf(debug_file, "<TABLE BORDER><TR><TH>Name<TH>Flags<TH>Passwd<TH>size");
    fprintf(debug_file, "<TH>Mode<TH>Uptime<TH>ChanOp</TR>\n");
    for (tmplink = bot->chan_list; tmplink ; tmplink = tmplink->next) {
	cell = tmplink->cell;
	if (tmplink->type == CELL_CHANS)
	    fprintf(debug_file,"<TR><TD>%s<TD>%d<TD>%s<TD>%d<TD>%d<TD>%ds<TD>%s</TR>\n",
		    ((chans *)cell)->name, ((chans *)cell)->flags,
		    (((chans *)cell)->passwd ? ((chans *)cell)->passwd : "(none)"),
		    ((chans *)cell)->size, ((chans *)cell)->mode,
		    (int)difftime(time((time_t *)NULL),((chans *)cell)->uptime),
		    ((chans *)cell)->iamchanop ? "Op" : "Not op");
	else
	    fprintf(debug_file,"<TR><TH COLSPAN=5>ERROR! CELL_TYPE %d</TR>\n", tmplink->type);
    }
    fprintf(debug_file, "<TR><TH>Name<TH>Flags<TH>Passwd<TH>size");
    fprintf(debug_file, "<TH>Mode<TH>Uptime<TH>ChanOp</TR>\n</TABLE>\n");

    fprintf(debug_file, "<H1>Liste des personnes en split</H1>\n");
    fprintf(debug_file, "<TABLE BORDER><TR><TH>Nick<TH>Addr<TH>chan<TH>Uptime</TR>\n");
    for (tmplink = bot->hack_list; tmplink ; tmplink = tmplink->next) {
	cell = tmplink->cell;
	if (tmplink->type == CELL_HACKS)
	    fprintf(debug_file,"<TR><TD>%s<TD>%s<TD>%s<TD>%d</TR>\n",
		    ((hacks *)cell)->nick, ((hacks *)cell)->addr,
		    ((hacks *)cell)->chan, (int)difftime(time((time_t *)NULL), ((hacks *)cell)->uptime));
	else
	    fprintf(debug_file,"<TR><TH COLSPAN=4>ERROR! CELL_TYPE %d</TR>\n", tmplink->type);
    }
    fprintf(debug_file, "<TR><TH>Nick<TH>Addr<TH>chan<TH>Uptime</TR>\n</TABLE>\n");
        
    fprintf(debug_file, "</BODY></HTML>");
    fclose(debug_file);
}
    
    
    

		    
    
