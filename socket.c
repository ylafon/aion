/* (c) Yves Lafon <yves@raubacapeu.net> */
/*                                      */
/* $id */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#ifdef NODELAY
#include <netinet/tcp.h>
#endif /* NODELAY */
#include <netdb.h>
#include <fcntl.h>
#include "defs.h"
#include "strip.h"
#include "socket.h"

#ifdef _AION_USE_SIGNAL_
#include "signal.h"
#endif /* _AION_USE_SIGNAL_ */

extern int h_errno;

int Connection(server, port)
    char *server;
    int port;
{
    int sock;
    struct hostent *hptr;
    struct sockaddr_in adsock = {AF_INET};
#ifdef NODELAY
    int val = 1;
#endif /* NODELAY */

    alarm(CONNECT_MAX_TIME);

    printf("Trying to connect to %s on port %d\n",server,port);
    if ((hptr=gethostbyname(server))==NULL) {
	perror("Unable to get host");
	return 0;
    }
#ifdef SUNOS
    bcopy((char *)hptr->h_addr, (char *)&adsock.sin_addr, hptr->h_length);
#else
    memcpy((void *)&adsock.sin_addr, (void *)hptr->h_addr, (unsigned int)hptr->h_length);
#endif /* SUNOS */
    adsock.sin_family=hptr->h_addrtype;
    adsock.sin_port=htons((unsigned int)port);
    if ((sock=socket(AF_INET,SOCK_STREAM,0))<0) {
	perror("Unable to create socket");
	return 0;
    }
	
    setsockopt(sock, SOL_SOCKET, SO_LINGER, 0, 0);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, 0, 0);
#ifdef NODELAY
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&val, sizeof(val));
#endif /* NODELAY */
    fcntl(sock, F_SETFL, O_NONBLOCK);
    if (connect(sock,(struct sockaddr *)&adsock,sizeof(adsock))<0) {
	if (errno==EINTR) {
	    perror("Timed out");
	} else {
	    if (errno == EINPROGRESS) {
		alarm(0);
		return sock;
	    }
	    perror("Unable to connect");
	}
	close(sock);
	return 0;
    }
    printf("Connected !\n");
    alarm(0);
    if (!sock)
	printf("opened descriptor NULL\n");
    return(sock);
}

int SendSocket(sock, msg)
    int sock;
    char *msg;
{
#ifdef DEBUG
    printf("-> %s",msg);
#endif 
    return((write(sock, msg, strlen(msg)) > 0));
}

int ReadSocket(sock, buffer, curr, result)
    int sock;
    char *buffer,*result;
    char **curr;
{
    register char *npos, *cpy, *cpyresult;
    int nb_read, s_size;
    register int i;

    errno=0;
    nb_read=read(sock, *curr, (unsigned int)(1022+buffer-*curr));

#ifdef _AION_USE_SIGNAL_
    if (errno==EINTR) 
	signal(SIGALRM,alarm_handler);
#endif /* _AION_USE_SIGNAL_ */

    if (!nb_read)
	return 0;
    else {
	*(*curr+nb_read)=0;
	npos=buffer;
	cpy =buffer;
#ifdef STRICT_DELIMITER
	while (*cpy)
	    if (*cyp++ == '\r')
		if (*cpy++ == '\n')
		    npos = cpy;
#else
	while (*cpy)
	    if (*cpy++ == '\n')
		npos = cpy;
#endif /* STRICT_DELIMITER */
 
#ifdef STRICT_DELIMITER
	s_size = (npos-buffer-2);
#else
	s_size = (npos-buffer-1);
#endif
	cpy = buffer;
	cpyresult = result;
	for (i=0;i<s_size;i++)
	    *cpyresult++ = *cpy++;
	*cpyresult=0;

	cpy = buffer;
	while (*npos)
	    *cpy++ = *npos++;
	*curr=cpy;;
	return 1;
    }
} 

char *NameToIp(name)
    char *name;
{
    struct hostent *host_info;
    char buffer[512];
    char *result;
    char *ip;
    int i;

    if (IsValidName(name)) {
	host_info = gethostbyname(name);
	
	if (!host_info) {
	    if (h_errno == HOST_NOT_FOUND) {
		result = (char *)malloc(strlen(name) + 16);
		sprintf(result,"Host %s not found", name);
		return result;
	    }
 
	    if (h_errno == NO_DATA) {
		result = (char *)malloc(strlen(name) + 64);
		sprintf(result,"The requested name %s is valid but does not have an IP address", name);
		return result;
	    }
	    
	    result = (char *)malloc(strlen(name) + 32);
	    sprintf(result,"Unrecoverable Error for %s", name);
	    return result;
	}
	
	result = buffer;
	sprintf(result, "%s:", name);
	result += strlen(result);
	i = 0;
	while (host_info->h_addr_list[i]) {
	    ip = host_info->h_addr_list[i];
	    sprintf(result, " %u.%u.%u.%u", *ip & 0xFF , *(ip+1) & 0xFF, *(ip+2) & 0xFF, *(ip+3) & 0xFF);
	    result += strlen(result);
	    i++;
	}
	result = (char *)malloc(strlen(buffer)+1);
	strcpy(result, buffer);
	return result;
    }
    result = (char *)malloc(16);
    strcpy(result,"Invalid Name");
    return result;

}
		
char *IpToName(ip_addr)
    char *ip_addr;
{
    struct hostent *host_info;
    struct hostent *ip_info;

    char buffer[512];
    char *result;
    int i;

    if (IsIp(ip_addr)) {
	ip_info = gethostbyname(ip_addr);

	if (!ip_info) {
	    result = (char *)malloc(strlen(ip_addr) + 32);
	    sprintf(result,"Unrecoverable Error for %s", ip_addr);
	    return result;
	} 

	host_info = gethostbyaddr(ip_info->h_addr_list[0], ip_info->h_length, ip_info->h_addrtype);

	if (!host_info) {
	    if (h_errno == HOST_NOT_FOUND) {
		result = (char *)malloc(strlen(ip_addr) + 16);
		sprintf(result,"Host %s not found", ip_addr);
		return result;
	    }
	    
	    result = (char *)malloc(strlen(ip_addr) + 32);
	    sprintf(result,"Unrecoverable Error for %s", ip_addr);
	    return result;
	}
	
	result = buffer;
	sprintf(result, "%s: %s", ip_addr, host_info->h_name);
	result += strlen(result);

	i = 0;
	while (host_info->h_aliases[i]) {
	    sprintf(result, " %s", host_info->h_aliases[i]);
	    result += strlen(result);
	    i++;
	}
	result = (char *)malloc(strlen(buffer)+1);
	strcpy(result, buffer);
	return result;
    }

    result = (char *)malloc(17);
    strcpy(result,"Invalid Address");
    return result;    
}
    
    
