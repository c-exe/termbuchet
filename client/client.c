#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>

#include "../sharedfuncs.h"
#include "client.h"

int running = 0;

int isrunning()
{
  return running;
}

void handle_sigint(int sig)
{
  running = 0;
  printf("Terminating.\n");
}

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) return &(((struct sockaddr_in *) sa)->sin_addr);
  return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

void dostop()
{
  running = 0;
}

int dorun()
{
  if (signal(SIGINT, handle_sigint) == SIG_ERR)
  {
    perror("Sigint registration error");
    return 0;
  }
  else running = 1;
  
  return 1;
}

int connecttohost(int *socketfd, char *hostipaddress, char *hostport, char *hostaddrout, int hostaddroutsize)
{
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  
  memset(&hints, 0, sizeof(addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  
  if ((rv = getaddrinfo(hostipaddress, hostport, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "Error getting address info: %s\n", gai_strerror(rv));
    return -1;
  }
  
  for (p = servinfo; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      perror("Socket error");
      continue;
    }
    
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
    {
      close(sockfd);
      perror("Error connecting");
      continue;
    }
    
    break;
  }
  
  if (p == NULL)
  {
    fprintf(stderr, "Failed to connect to %s:%s!\n", hostipaddress, hostport);
    freeaddrinfo(servinfo);
    return -2;
  }
  
  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), hostaddrout, hostaddroutsize);
  
  freeaddrinfo(servinfo);
  
  (*socketfd) = sockfd;
  
  return 1;
}

int fullsend(int socketfd, char *senddata, int datalen, int flags)
{
  int n, total = 0;
  int remaining = datalen;
  
  while (total < datalen)
  {
    n = send(socketfd, senddata+(sizeof(char)*total), remaining, flags);
    if (n == -1) break;
    total += n;
    remaining -= n;
  }
  
  if (n == -1) return n;
  return total;
}

