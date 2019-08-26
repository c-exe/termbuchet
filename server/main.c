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

#include "server.h"

int main(int argc, char *argv[])
{
  
  int listener;
  
  if ((listener = setupconnection(PORT_STR)) < 0)
  {
    fprintf(stderr, "Error: Could not start server!")
    exit(1);
  }
  
  
  
  while (isrunning())
  
}