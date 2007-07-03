/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#ifndef _AION_DEFS_H_
#define _AION_DEFS_H_

#ifdef PARAM1
#  undef PARAM1
#endif /* PARAM1 */
#ifdef PARAM2
#  undef PARAM2
#endif /* PARAM2 */
#ifdef PARAM3
#  undef PARAM3
#endif /* PARAM3 */
#ifdef PARAM4
#  undef PARAM4
#endif /* PARAM4 */

#ifdef __STDC__
#  define PARAM1(a) (a)
#  define PARAM2(a,b) (a,b)
#  define PARAM3(a,b,c) (a,b,c)
#  define PARAM4(a,b,c,d) (a,b,c,d)
#  define PARAM5(a,b,c,d,e) (a,b,c,d,e)
#  define PARAM6(a,b,c,d,e,f) (a,b,c,d,e,f)
#else
#  define PARAM1(a) ()
#  define PARAM2(a,b) ()
#  define PARAM3(a,b,c) ()
#  define PARAM4(a,b,c,d) () 
#  define PARAM5(a,b,c,d,e) ()
#  define PARAM6(a,b,c,d,e,f) ()
#  define const
#endif /* __STDC__ */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#define CELL_HOSTS 0
#define CELL_NICKS 1
#define CELL_USERS 2
#define CELL_CHANS 3
#define CELL_HACKS 4
#define CELL_WHO   5
#define CELL_BOT   6
#define CELL_TODOS 7
#define CELL_UH    8
#define CELL_BAN   9

#define STATE_NOT_CONNECTED 0
#define STATE_NOT_REGISTERED 1
#define STATE_RUNNING 2

#define ACTION_NICK 0
#define ACTION_CHAN 1

#define ETERNITY 9999999
#define HACK_MAX_TIME 1800
#define MAX_PING 600
#define HACK_REOP_TIME 10
#define REVENGE_TIME 180
#define LAME_FAKE_TIME 120
#define LAME_FAKE_TIME_SKEW 10
#define HACK_REOP_TIME_SKEW 5
#define CHAN_MODE_ON_JOIN_DELAY 20
#define TOPIC_ON_JOIN_DELAY CHAN_MODE_ON_JOIN_DELAY+1
#define MASTER_TIME 300
#define REJOIN_TIME 30
#define MODE_K_TIME 5
#define WAR_TIME 0
#define LOOP_NICK_TIME ETERNITY
#define REDO_MODE_TIME ETERNITY
#define USER_PREFIXES "~^+=-" /* user prefixes in 2.9 server */
#define USER_PREFIX_NUM 5

#define US_MASTER 1
#define US_FRIEND 2
#define US_USER   4
#define US_BOT    8
#define US_AUTOOP 16
#define US_KPRO   32
#define US_OPRO   64
#define US_BPRO   128
#define US_SHIT   256
#define US_REBAN  512

#define BOT_COMMAND_CHAR '\\'

#define C_REOP     0
#define C_DEOP     1
#define C_KICK     2
#define C_BAN      3
#define C_KBAN     4
#define C_TBAN     5
#define C_TKBAN    6
#define C_ADDOP    7
#define C_ADDUSER  8
#define C_ADDSHIT  9
#define C_ADDBAN   10
#define C_ADDOPRO  11
#define C_ADDKPRO  12
#define C_DELOP    13
#define C_DELUSER  14
#define C_DELSHIT  15
#define C_DELBAN   16
#define C_DELOPRO  17
#define C_DELKPRO  18
#define C_DEBAN    19
#define C_WHOIS    20
#define C_USERINFO 21
#define C_NOTHING  0
#define C_MASSOP   1
#define C_RESYNCH  2
#define C_MASSDOP  3
#define C_MASSKICK 4

#define KEEP_L   1
#define KEEP_O   2
#define KEEP_B   4
#define KEEP_K   8
#define KEEP_I  16
#define KEEP_P  32
#define KEEP_S  64
#define KEEP_N 128
#define KEEP_M 256
#define KEEP_T 512
#define KEEP_CHAN 1017
#define KEEP_ALL 1023

#define PLUS_L   1
#define PLUS_K   2
#define PLUS_I   4
#define PLUS_P   8
#define PLUS_S  16
#define PLUS_N  32
#define PLUS_M  64
#define PLUS_T 128
#define PLUS_C 256 /* colorless ? */

#define REVENGE 2048
#define NO_HACK 4096

#define LOCK_TOPIC 8192
#define FORCE_MODE 16384
#define LOG_CHAN   32768
#define LOGTIME_GMT 65536

#define GATEWAY_LOCAL_CHAN 131072

/* Behaviour mask */
#define B_CLIENT                  1
#define B_SERVER                  2
#define B_SERVICE                 4
#define B_LOOP_ON_ERROR           8
#define B_LOOP_NICK_ON_COLL      16
#define B_LOOP_SERV_ON_COLL      32
#define B_LOOP_NICK_ON_KILL      64
#define B_LOOP_SERV_ON_KILL     128
#define B_CHECK_CHANNEL_TIMEOUT 256

/* every two minutes */
#define CHANNEL_DELAY_TIMEOUT 120

#define MAX_MODES 3
#define POPSTR "+ooo"
#define MOPSTR "-ooo"
#define MAX_NOTICES 8
#define MAX_LOGIN_LENGTH 8
#define ENDCLASS_A 128
#define ENDCLASS_B 192
#define ENDCLASS_C 240
#define ENDCLASS_D 256

#define FLAG_RANDOM_NICK_FIRST 1
#define FLAG_ADD_COMMAND_CHAR  2

#define CONNECT_MAX_TIME 20

#endif /* _AION_DEFS_H_ */


