/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "time.h"

#ifdef SUNOS
double difftime(time1, time2)
    time_t time1,time2;
{
    return((double)time1-time2);
}
#endif

void GmtTime(timestring)
char *timestring;
{
    char f1[16],f2[16],f3[16],f4[16],f5[16];
    time_t tim;
    struct tm *time_tm;
    
    tim=time((time_t *)NULL);
    time_tm=gmtime(&tim);
    sscanf(asctime(time_tm),"%s %s %s %s %s",f1,f2,f3,f4,f5);
    if (strlen(f3) == 1) 
	sprintf(timestring,"%s, 0%s %s %s %s GMT",f1,f3,f2,f5,f4);
    else
	sprintf(timestring,"%s, %s %s %s %s GMT",f1,f3,f2,f5,f4);
}

void LocalTime(timestring)
char *timestring;
{
      char f1[16],f2[16],f3[16],f4[16],f5[16];
      time_t tim;
      struct tm *time_tm;
      
      tim=time((time_t *)NULL);
      time_tm=localtime(&tim);
      sscanf(asctime(time_tm),"%s %s %s %s %s",f1,f2,f3,f4,f5);
      if (strlen(f3) == 1) 
	  sprintf(timestring,"%s, 0%s %s %s %s",f1,f3,f2,f5,f4);
      else
	  sprintf(timestring,"%s, %s %s %s %s",f1,f3,f2,f5,f4);
}  

void ShortTime(timestring)
char *timestring;
{
      char f2[16],f3[16],f4[16];
      time_t tim;
      struct tm *time_tm;
      
      tim=time((time_t *)NULL);
      time_tm=localtime(&tim);
      sscanf(asctime(time_tm),"%*s %s %s %s",f2,f3,f4);
      sprintf(timestring,"%s %s %s",f3,f2,f4);
}  
