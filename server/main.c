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
#include <sqlite3.h>

#include "../sharedfuncs.h"
#include "server.h"

fd_set master, read_fds;
int fdmax;


int main(int argc, char *argv[])
{
  
  int listener;
  char remoteip[INET6_ADDRSTRLEN];
  
  if ((listener = setupconnection(PORT_STR)) < 0)
  {
    fprintf(stderr, "Error: Could not start server!")
    exit(1);
  }
  
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  
  fdmax = listener;
  FD_SET(listener, &master);
  FD_SET(STDINFD, &master);
  
  if (dolisten(int listener) < 0)
  {
    fprintf(stderr, "Error: Could not listen for connections!")
    closeconnections(listener);
    exit(2);
  }
  
  while (isrunning())
  {
    read_fds = master;
Do_Select_Restart:
    if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
    {
      if (errno == EINTR)
      {
        if (isrunning()) goto Do_Select_Restart;
        break;
      }
      perror("Error Selecting");
      break;
    }
    
  }
  
  closeconnections(listener);
}