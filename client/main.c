#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../sharedfuncs.h"
#include "client.h"

#define MAXBACKBUFFER 4096

char hport[31] = PORT_STR;
char backbuffer[MAXBACKBUFFER] = "";
int backbufferend = 0;
int swidth = 0, sheight = 0;

void getscreensize()
{
  struct winsize ws;
  char output[256] = "";
  int i;
  if (ioctl(1,TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
  {
    //Can't get it the proper way, try the old-fashioned one...
    int prevx = 0, prevy = 0;
    write(SCREENFD,"\033[6n",4);
    while (i<255)
    {
      if (read(STDINFD,output+(i*sizeof(char)),1) != 1) break;
      if (output[i] == 'R') break;
      i++;
    }
    output[i] = 0;
    sscanf(output,"\033[%lu;%lu",prevy,prevx);
    write(SCREENFD,"\033[999;999H", "\033[999;999H");
    write(SCREENFD,"\033[6n",4);
    while (i<255)
    {
      if (read(STDINFD,output+(i*sizeof(char)),1) != 1) break;
      if (output[i] == 'R') break;
      i++;
    }
    output[i] = 0;
    sscanf(output,"\033[%lu;%lu",sheight,swidth);
    snprintf(output, 255, "\033[%d;%dH",prevy,prevx);
    write(SCREENFD,output, strlen(output));
  }
  else
  {
    swidth = ws.ws_col;
    sheight = ws.ws_row;
  }
}

int proccmd(int socketfd)
{
  int offset = 0, rv;
  int inpacket = 0;
  char tempstr[256] = "";
  backbuffer[backbufferend] = 0;
  while (offset < backbufferend)
  {
    if (strstartsas(backbuffer+(offset*sizeof(char)), MSG_SEPARATOR))
    {
      offset += strlen(MSG_SEPARATOR);
      continue;
    }
    else if (strstartsas(backbuffer+(offset*sizeof(char)), MSG_REQUEST))
    {
      offset += strlen(MSG_REQUEST);
      //Deal with request
      if (strstartsas(backbuffer+(offset*sizeof(char)), MSG_COLOURTYPE MSG_SEPARATOR))
      {
        //Find out colourtype somehow and return it!
      }
      else if (strstartsas(backbuffer+(offset*sizeof(char)), MSG_WIDTH MSG_SEPARATOR))
      {
        //Find out the width and return it...
        getscreensize();
        snprintf(tempstr, 255, "%s%s%d%s", MSG_WIDTH, MSG_IS, swidth, MSG_SEPARATOR);
        fullsend(socketfd, tempstr, strlen(tempstr)*sizeof(char), 0);
        offset += strlen(MSG_WIDTH MSG_SEPARATOR);
      }
      else if (strstartsas(backbuffer+(offset*sizeof(char)), MSG_HEIGHT MSG_SEPARATOR))
      {
        //Find out the Height and return it...
        getscreensize();
        snprintf(tempstr, 255, "%s%s%d%s", MSG_HEIGHT, MSG_IS, sheight, MSG_SEPARATOR);
        fullsend(socketfd, tempstr, strlen(tempstr)*sizeof(char), 0);
        offset += strlen(MSG_HEIGHT MSG_SEPARATOR);
      }
      else
      {
        if ((rv = strstr(backbuffer+(offset*sizeof(char)), MSG_SEPARATOR)) >= 0)
        {
          fprintf(stderr, "Warning: Malformed Request from server!\n");
          fullsend(socketfd, MSG_MSGERROR, strlen(MSG_MSGERROR)*sizeof(char), 0);
          offset += rv;
        }
        else
        {
          break;
        }
      }
    }
    else if (strstartsas(backbuffer+(offset*sizeof(char)), MSG_CMD))
    {
      offset += strlen(MSG_CMD);
      //Deal with command
    }
    else if (strstartsas(backbuffer+(offset*sizeof(char)), MSG_RECEIVED))
    {
      offset += strlen(MSG_RECEIVED);
      //Ignore this?
    }
    else
    {
      break;
    }
  }
  
}

int main(int argc, char *argv[])
{
  char *buffer[BUFFERSIZE];
  int rv, numbytes;
  char hostip[INET6_ADDRSTRLEN+1];
  int socketfd;
  
  if (argc <2 || argc > 3)
  {
    fprintf(stderr, "Usage:\n%s hostname [hostport]\nIf hostport isn't specified, it defaults to %s.\n", argv[0], PORT_STR);
    exit(1);
  }
  if (argc>2) strncpy(hport, 30, argv[2]);
  
  if (connecttohost(&socketfd, argv[1], hport, hostip, INET6_ADDRSTRLEN) < 1)
  {
    exit(2);
  }
  
  if (!dorun())
  {
    close(socketfd);
    exit(3);
  }
  
  while (isrunning())
  {
    if ((numbytes = recv(socketfd, buffer, BUFFERSIZE-1, 0)) == -1)
    {
      perror("Receiving error!")
      dostop();
      break;
    }
    buffer[numbytes] = 0;
    if (numbytes + backbufferend < MAXBACKBUFFER)
    {
      memcpy(backbuffer + (sizeof(char)*backbufferend), buffer, numbytes);
      backbufferend += numbytes;
      buffer[0] = 0;
    }
    proccmd(socketfd);
  }
  
  close(socketfd);
  
  exit(0);
}