/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $Id: signal.c,v 1.2 2007/07/03 14:02:58 ylafon Exp $ */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/signal.h>
#include "signal.h"

void IgnoreAll()
{
/*  signal(SIGINT,SIG_IGN); */
  signal(SIGHUP,SIG_IGN);
  signal(SIGILL,SIG_IGN);
  signal(SIGFPE,SIG_IGN);
  signal(SIGBUS,SIG_IGN);
  signal(SIGSEGV,SIG_IGN);
  signal(SIGPIPE,SIG_IGN);
  signal(SIGTERM,SIG_IGN);
  signal(SIGSTOP,SIG_IGN);
  signal(SIGUSR1,SIG_IGN);
  signal(SIGUSR2,SIG_IGN);
  signal(SIGABRT,SIG_IGN);
  signal(SIGALRM,IgnoreAll);
}
