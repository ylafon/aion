/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#ifndef _AION_CELL_H_
#define _AION_CELL_H_

#include "defs.h"
#include "types.h"
#include <time.h>

void FreeCell PARAM2(int, void *);
chans *CreateChannel PARAM5(char *, int, char *, int, int);
who   *CreateWho PARAM3(char *, char *, char *);
hacks *CreateHack PARAM3(char *, char *, char *);
todos *CreateTodo PARAM2(char *, int);
users *CreateUser PARAM3(char *, char *, int);
void UpdateUser PARAM6(list **, char *, char *, int, int, int PARAM2(void *, void *));
void UpdateUser ();
nicks *CreateNick PARAM1(char *);
hosts *CreateHost PARAM2(char *, int);
hosts *CreateHostPass PARAM3(char *, int, char *);
uh *CreateUh PARAM4(int, char *, char *, int);
ban *CreateBan PARAM2(char *, char *);
void PurgeUserList PARAM1(list **);
void PurgeTodoList PARAM2(irc_bot *, list **);
void PurgeHackList PARAM2(irc_bot *, list **);

#endif /* _AION_CELL_H_ */
