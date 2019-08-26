#ifndef SERVER_INCLUDED
#define SERVER_INCLUDED 1

#define PORT_STR "9034"
#define PORT_N 9034

#define BUFFERSIZE 512

typedef enum colourtype_enum
{
  monochrome, //1 bit = 2 colour
  eight,      //3 bit = 8 colour
  sixteen,    //4 bit = 16 colour
  byte,       //8 bit = 256 colour
  truecolour  //24 bit
} colourtype;

typedef struct displaynode_struct
{
  int socketfd;
  colourtype colours;
  unsigned long width;
  unsigned long height;
  char *metaformat;  //Format of the song/bible passage metadata
  char *ipaddress;
  struct displaynode_struct *next;
} displaynode;

displaynode *adddisplay(int socketfd);
displaynode *getdisplay(int socketfd);
int freedisplay(int socketfd);
int isrunning();
void handle_sigint(int sig);
void *get_in_addr(struct sockaddr *sa);
int setupconnection(char *port_str); //Returns listener fd
int closeconnections(int listener);
int dolisten(int listener);
displaynode *doaccept(int listener);
int doclose(int socketfd);


#endif