/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: types.h,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

#ifndef _AION_TYPES_H_
#define _AION_TYPES_H_

#include <stdio.h>
#include <time.h>

typedef struct general_list_str {
    int type;
    void *cell;
    struct general_list_str *next;
    struct general_list_str *prev;
} list;

typedef struct hosts_ {
    char *name;
    char *passwd;
    int port;
    time_t uptime;
} hosts;

typedef struct nick_ {
    char *nick;
    time_t uptime;
} nicks;

typedef struct user_host_ {
    int  type;
    char *chan;
    char *nick;
    int  timer;
} uh;

typedef struct ban_ {
    char *chan;
    char *addr;
} ban;

typedef struct user_lists_ {
    char *addr;
    char *channel;
    time_t uptime;
    int actions;
    int bad_actions;
    int flags;
    time_t ms_when;
    int    ms_time;
    time_t fr_when;
    int    fr_time;
    time_t us_when;
    int    us_time;
    time_t bot_when;
    int    bot_time;
    time_t op_when;
    int    op_time;
    time_t kpro_when;
    int    kpro_time;
    time_t opro_when;
    int    opro_time;
    time_t bpro_when;
    int    bpro_time;
    time_t sh_when;
    int    sh_time;
    time_t reban_when;
    int    reban_time;
} users;

typedef struct chan_list_ {
    char *name;
    char *topic;
    int flags;
    char *passwd;
    FILE *log;
    int size;
    int mode;
    time_t uptime;
    time_t last_mode;
    int iamchanop;
} chans;

typedef struct hack_list_ {
    char *chan;
    char *nick;
    char *addr;
    time_t uptime;
} hacks;

typedef struct who_list_ {
    char *nick;
    char *addr;
    char *chan;
    time_t uptime;
    int chanop;
} who;

typedef struct todos_list_ {
    char   *tobedone;
    time_t when;
    int    todo_time;
} todos;

typedef struct irc_bot_ {
    int state;
    int socknum;

    char sockbuffer[1024];
    char *sockbufferpos;
    char readfromsocket[1024];

    int quit;
    int verbose;
    int nb_notice;
    int last_action; /* for CD/ND incoherence */
    FILE *outfile;
    int behaviour;
    char *logname;
    char *configname;
    time_t uptime;
    time_t s_ping;
    int config_flag;
    time_t join_timestamp; /* for CD/ND */

    char *username;
    char *realname;
    char cmd_char;

    int hack_max_time;
    int max_ping;
    int hack_reop_time;
    int revenge_time;
    int lame_fake_time;
    int loop_nick_time;
    int redo_mode_time;
    int war_time;
    int rejoin_time;

    list *host_list;  /* general bot structure */
    list *nick_list;

    list *us_list;
    list *war_list;
    list *who_list;  

    list *db_request; /* request lists for mode b and userhost */
    list *uh_request;

    list *ban_list;
    list *chan_list;
    list *todo_list;
    list *hack_list;
} irc_bot;

#endif /* _AION_TYPES_H_ */

