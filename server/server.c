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

#include "server.h"

int running = 0;

displaynode *dnroot = NULL, *dnptr = NULL;

displaynode *adddisplay(int socketfd)
{
  displaynode *adn = (displaynode *) malloc(sizeof(displaynode));
  if (!adn) return NULL;
  memset(adn, 0, sizeof(displaynode));
  adn->socketfd = socketfd;
  adn->next = NULL;
  if (!dnroot)
  {
    dnroot = adn;
  }
  else
  {
    for (dnptr = dnroot; dnptr->next != NULL; dnptr = dnptr->next);
    dnptr->next = adn;
  }
  return adn;
}

displaynode *getdisplay(int socketfd)
{
  for (dnptr = dnroot; dnptr != NULL; dnptr = dnptr->next)
  {
    if (dnptr->socketfd == socketfd) return dnptr;
  }
  return NULL;
}

int freedisplay(int socketfd)
{
  displaynode *adn;
  for (dnptr = dnroot; dnptr->next != NULL; dnptr = dnptr->next);
  {
    if (dnptr->next->socketfd == socketfd)
    {
      adn = dnptr->next;
      dnptr->next = adn->next;
      if (adn->metaformat != NULL) free(adn->metaformat);
      if (adn->ipaddress != NULL) free(adn->ipaddress);
      free(adn);
      return 1;
    }
  }
  return 0;
}

int isrunning()
{
  return running;
}

void handle_sigint(int sig)
{
  running = 0;
  printf("Termination initiated.\r\n");
}

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) return &(((struct sockaddr_in *) sa)->sin_addr);
  return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int setupconnection(char *port_str)
{
  //Returns a file descriptor to listen on
  int listener;
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;
  
  int yes=1;
  
  struct addrinfo hints, *addrinfo, *p;
  
  int rv;
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  
  if ((rv = getaddrinfo(NULL, port_str, &hints, &addrinfo)) != 0)
  {
    fprintf(stderr, "Error getting socket: %s\n", gai_strerror(rv));
    return -1;
  }
  
  for (p = addrinfo; p != NULL; p = p->ai_next)
  {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0) continue;
    
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    
    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
    {
      close(listener);
      continue;
    }
    
    break;
  }
  
  if (p == NULL)
  {
    fprintf(stderr, "Error: Failed to bind socket\n");
    freeaddrinfo(addrinfo);
    return -2;
  }
  
  freeaddrinfo(addrinfo);
  
  return listener;
}

int closeconnections(int listener)
{
  close(listener);
  //Close all connections too!
  while (dnroot != NULL)
  {
    //Send a close warning!
    //then close the socket
    close(dnptr->socketfd);
    //then delete it
    dnptr = dnroot->next;
    free(dnroot);
    dnroot = dnptr;
  }
  //then
  running = 0;
  return 1;
}

int dolisten(int listener)
{
  if (listen(listener, 10) == -1)
  {
    perror("Error listening");
    return -1;
  }
  
  if (signal(SIGINT, handle_sigint) == SIG_ERR)
  {
    perror("Sigint registration error");
    return -2;
  }
  else running = 1;
  
  return 1;
}

displaynode *doaccept(int listener)
{
  displaynode *adn = NULL;
  int newfd;
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;
  
  addrlen = sizeof(remoteaddr);
  newfd = accept(listener, (struct sockaddr *) &remoteaddr, &addrlen);
  if (newfd == -1)
  {
    return NULL;
  }
  else
  {
    adn = adddisplay(newfd);
    if (!adn)
    {
      close(newfd);
      return NULL;
    }
    adn->ipaddress = (char *) malloc(sizeof(char)*(1+INET6_ADDRSTRLEN));
    if (adn->ipaddress)
    {
      //If that worked, fill it in.
      inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *) &remoteaddr), adn->ipaddress, INET6_ADDRSTRLEN);
    }
  }
  return adn;
}

int doclose(int socketfd)
{
  if (close(socketfd))
  {
    //perror("Close Error");
    return 0;
  }
  if (!freedisplay(socketfd)) return 0;
  return 1;
}
