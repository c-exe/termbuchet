#ifndef SERVER_INCLUDED
#define SERVER_INCLUDED 1

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT_STR "9034"
#define PORT_N 9034

#define BUFFERSIZE 512

int isrunning();
void handle_sigint(int sig);
void *get_in_addr(struct sockaddr *sa);
void dostop();
int dorun();
int connecttohost(int *socketfd, char *hostipaddress, char *hostport, char *hostaddrout, int hostaddroutsize);
int fullsend(int socketfd, char *senddata, int datalen, int flags);


#endif