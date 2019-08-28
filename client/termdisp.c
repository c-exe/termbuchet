#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../sharedfuncs.h"
#include "termstyles.h"
#include "termdisp.h"

stylesblock normalstyle, currentstyle;
int forcedcolourtypespec = 0;
int colourtypespec = 0;

void revertcurrenttonormal()
{
  memcpy(currentstyle, normalstyle, sizeof(stylesblock));
  write(SCREENFD, STYLESTART PLAINTEXT STYLEEND, sizeof(char)*(N_STYLESTART+N_PLAINTEXT+N_STYLEEND));
  if (currentstyle.isbold) write(SCREENFD, STYLESTART BRIGHTTEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_BRIGHTTEXTON+N_STYLEEND));
  if (currentstyle.isitalics) write(SCREENFD, STYLESTART ITALICTEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_ITALICTEXTON+N_STYLEEND));
  if (currentstyle.isunderlined) write(SCREENFD, STYLESTART UNDERLINETEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_UNDERLINETEXTON+N_STYLEEND));
  if (currentstyle.isshadowed)  {}
  switch (currentstyle.outlinecolour) 
  {
    case 1:
    break;
    
    case 2:
    break;
    
    case 3:
    break;
  }
  //Then deal with colours
}

void setforcedcolourtypespec(int colourtype)
{
  forcedcolourtypespec = colourtype;
}

int runexternal(char *outputstr, int outputstr_size, char *program, char *arg1)
{
  //A quick bodge for running external programs with one argument
  int outpipe[2];
  pid_t apid
  char buffer[MAXBUFFERSIZE];
  int rdn = MAXBUFFERSIZE-1;
  int n, xn = outputstr_size, xon = 0;
  if (rdn>outputstr_size-1) rdn = outputstrsize-1;
  pipe(outpipe);
  apid = fork();
  switch (apid)
  {
    case -1:
      return -1;
    break;
    
    case 0:
      //child
      close(outpipe[0]);
      dup2(outpipe[1],SCREENFD);
      execlp(program, program, arg1);
      //If we got here, it failed
      write(SCREENFD,"\004",1); //Send an EOF
      exit(1);
      _exit(1);
    break;
    
    default:
      //parent
      close(outpipe[1]);
      waitpid(apid, NULL, 0); //I should really have a timer to kill it if it goes on too long
      do
      {
        n = read(outpipe[0], buffer, rdn);
        if (n>0 && buffer[0]==4) break;
        buffer[rdn] = 0;
        if (n>0) memcpy(outputstr + (sizeof(char)*xon), buffer, rdn);
        xn -= n;
        xon += n;
        if (xn <= 0) break;
        rdn = MAXBUFFERSIZE-1;
        if (rdn > xn) rdn = xn;
      } while(n>0);
      close(outputpipe[0]);
    break;
    if (xon>0 && outputstr[0]==4) return -2;
  }
  return 1;
}

int updatecolourtypespec()
{
  //Returns: 1=Mono, 3=8 colours, 4=16 colours, 8=256 colours, 24=truecolour, 0=unknown
  
  //Check if colour type has been forced to a value and return if so
  if (forcedcolourtypespec)
  {
    colourtypespec = forcedcolourtypespec;
    return colourtypespec;
  }
  //Otherwise check if it's a tty and if not say mono
  if (!isatty(SCREENFD))
  {
    colourtypespec = 1;
    return 1;
  }
  //Otherwise, we could look up the TERM in TERMINFO or TERMCAP (tricky, bloated and it turns out it's not definitive anyway!), or...
  const char *tt = getenv("TERM");
  char theoutput[512]="";
  int numcols = 0;
  int rv = runexternal(theoutput, 512, "tput","colors"); //risky
  if (rv == 1) //It worked
  {
    sscanf(theoutput,"%d", &numcols);
    if (numcols == 8) colourtypespec = 3;
    else if (numcols == 16) colourtypespec = 4;
    else if (numcols == 256) colourtypespec = 8;
    else if (numcols > 256) colourtypespec = 24; //probably
    else if (numcols < 0) colourtypespec = 1; //usually
    if (colourtypespec) return colourtypespec;
  }
  rv = runexternal(theoutput, 512, "infocmp",tt); //riskier
  if (rv == 1) //It worked
  {
    char *colout = strstr("colors",theoutput);
    if (colout)
    {
      if (colout[6] == '@') colourtypespec = 1; //usually
      else if (colout[6] == '#')
      {
        char *comma = strstr(",",colout);
        if (comma)
        {
          comma[0] = 0;
          sscanf(theoutput,"colors#%d", &numcols);
          if (numcols == 8) colourtypespec = 3;
          else if (numcols == 16) colourtypespec = 4;
          else if (numcols == 256) colourtypespec = 8;
          else if (numcols > 256) colourtypespec = 24; //probably
        }
      }
    }
    if (colourtypespec) return colourtypespec;
  }
  
  //Use this if we still don't have an answer...
  if (strstr("-mono", tt) != NULL || strstr("-MONO", tt) != NULL)
  {
    //Convention says this is mono.
    colourtypespec = 1;
    return 1;
  }
  if (strstr("-256", tt) != NULL)
  {
    //Convention says this is 256 colours.
    colourtypespec = 8;
    return 8;
  }
  //I dunno!
  return 0;
}

int writestylefromtag(char *atag)
{
  /* Writes a style to SCREENFD based on the tag provided */
  if (strsame(atag,"[[") || strsame(atag,"[]")
  {
    //Escaped tag character
    write(SCREENFD, atag+sizeof(char), sizeof(char));
  }
  else if (strisame(atag,"[=N]"))
  {
    //Normal - TODO: need to get this from settings
    //For the moment, we'll assume Plain Text
    write(SCREENFD, STYLESTART PLAINTEXT STYLEEND, sizeof(char)*(N_STYLESTART+N_PLAINTEXT+N_STYLEEND));
  }
  else if (strisame(atag, "[=B=0]"))
  {
    //Bold Off (using bright)
    write(SCREENFD, STYLESTART BRIGHTTEXTOFF STYLEEND, sizeof(char)*(N_STYLESTART+N_BRIGHTTEXTOFF+N_STYLEEND));
    currentstyle.isbold = 0;
  }
  else if (strisame(atag, "[=B=1]"))
  {
    //Bold On (using bright)
    write(SCREENFD, STYLESTART BRIGHTTEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_BRIGHTTEXTON+N_STYLEEND));
    currentstyle.isbold = 1;
  }
  else if (strisame(atag, "[=B=N]"))
  {
    //Bold Normal (using bright) using normalstyle
    if (normalstyle.isbold && !currentstyle.isbold) 
    {
      write(SCREENFD, STYLESTART BRIGHTTEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_BRIGHTTEXTON+N_STYLEEND));
    }
    else if (!normalstyle.isbold && currentstyle.isbold) 
    {
      write(SCREENFD, STYLESTART BRIGHTTEXTOFF STYLEEND, sizeof(char)*(N_STYLESTART+N_BRIGHTTEXTOFF+N_STYLEEND));
    }
    currentstyle.isbold = normalstyle.isbold;
  }
  else if (strisame(atag, "[=I=0]"))
  {
    //Italics Off
    write(SCREENFD, STYLESTART ITALICTEXTOFF STYLEEND, sizeof(char)*(N_STYLESTART+N_ITALICTEXTOFF+N_STYLEEND));
    currentstyle.isitalics = 0;
  }
  else if (strisame(atag, "[=I=1]"))
  {
    //Italics On
    write(SCREENFD, STYLESTART ITALICTEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_ITALICTEXTON+N_STYLEEND));
    currentstyle.isitalics = 1;
  }
  else if (strisame(atag, "[=I=N]"))
  {
    //Italics Normal using normalstyle
    if (normalstyle.isitalics && !currentstyle.isitalics) 
    {
      write(SCREENFD, STYLESTART ITALICTEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_ITALICTEXTON+N_STYLEEND));
    }
    else if (!normalstyle.isitalics && currentstyle.isitalics) 
    {
      write(SCREENFD, STYLESTART ITALICTEXTOFF STYLEEND, sizeof(char)*(N_STYLESTART+N_ITALICTEXTOFF+N_STYLEEND));
    }
    currentstyle.isitalics = normalstyle.isitalics;
  }
  else if (strisame(atag, "[=U=0]"))
  {
    //Underline Off
    write(SCREENFD, STYLESTART UNDERLINETEXTOFF STYLEEND, sizeof(char)*(N_STYLESTART+N_UNDERLINETEXTOFF+N_STYLEEND));
    currentstyle.isunderlined = 0;
  }
  else if (strisame(atag, "[=U=1]"))
  {
    //Underline On
    write(SCREENFD, STYLESTART UNDERLINETEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_UNDERLINETEXTON+N_STYLEEND));
    currentstyle.isunderlined = 1;
  }
  else if (strisame(atag, "[=U=N]"))
  {
    //Underline Normal using normalstyle
    if (normalstyle.isunderlined && !currentstyle.isunderlined) 
    {
      write(SCREENFD, STYLESTART UNDERLINETEXTON STYLEEND, sizeof(char)*(N_STYLESTART+N_UNDERLINETEXTON+N_STYLEEND));
    }
    else if (!normalstyle.isunderlined && currentstyle.isunderlined) 
    {
      write(SCREENFD, STYLESTART UNDERLINETEXTOFF STYLEEND, sizeof(char)*(N_STYLESTART+N_UNDERLINETEXTOFF+N_STYLEEND));
    }
    currentstyle.isunderlined = normalstyle.isunderlined;
  }
  else if (strisame(atag,"[=SH=0]") || strisame(atag,"[=SH=1]") || strisame(atag,"[=SH=N]"))
  {
    //Ignore shadow for now
  }
  else if (strisame(atag,"[=OB=0]") || strisame(atag,"[=OB=1]") || strisame(atag,"[=OB=N]"))
  {
    //Ignore Black Border for now
  }
  else if (strisame(atag,"[=OW=0]") || strisame(atag,"[=OW=1]") || strisame(atag,"[=OW=N]"))
  {
    //Ignore White Border for now
  }
  else if (strisame(atag,"[=OI=0]") || strisame(atag,"[=OI=1]") || strisame(atag,"[=OI=N]"))
  {
    //Ignore Inverse Border for now
  }
  else if (strisame(atag, "[=C=Red]"))
  {
    //Red Text
    if (!colourtypespec) updatecolourtypespec();
    if (colourtypespec >= 4) write(SCREENFD, STYLESTART FGRED STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (colourtypespec >= 3) write(SCREENFD, STYLESTART FGMAROON STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Yellow]"))
  {
    //Yellow Text
    if (!colourtypespec) updatecolourtypespec();
    if (colourtypespec >= 4) write(SCREENFD, STYLESTART FGYELLOW STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (colourtypespec >= 3) write(SCREENFD, STYLESTART FGOLIVE STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Green]"))
  {
    //Green Text
    if (!colourtypespec) updatecolourtypespec();
    if (colourtypespec >= 4) write(SCREENFD, STYLESTART FGLIME STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (colourtypespec >= 3) write(SCREENFD, STYLESTART FGGREEN STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Blue]"))
  {
    //Blue Text
    if (!colourtypespec) updatecolourtypespec();
    if (colourtypespec >= 4) write(SCREENFD, STYLESTART FGBLUE STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (colourtypespec >= 3) write(SCREENFD, STYLESTART FGNAVY STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Cyan]"))
  {
    //Cyan Text
    if (!colourtypespec) updatecolourtypespec();
    if (colourtypespec >= 4) write(SCREENFD, STYLESTART FGCYAN STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (colourtypespec >= 3) write(SCREENFD, STYLESTART FGTEAL STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Magenta]"))
  {
    //Magenta Text
    if (!colourtypespec) updatecolourtypespec();
    if (colourtypespec >= 4) write(SCREENFD, STYLESTART FGFUCHSIA STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (colourtypespec >= 3) write(SCREENFD, STYLESTART FGMAGENTA STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Black]"))
  {
    //Black Text
    if (!colourtypespec) updatecolourtypespec();
    if (colourtypespec >= 3) write(SCREENFD, STYLESTART FGBLACK STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=White]"))
  {
    //White Text
    if (!colourtypespec) updatecolourtypespec();
    if (colourtypespec >= 4) write(SCREENFD, STYLESTART FGWHITE STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (colourtypespec >= 3) write(SCREENFD, STYLESTART FGSILVER STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Silver]"))
  {
    //White Text
    if (!colourtypespec) updatecolourtypespec();
    if (colourtypespec >= 3) write(SCREENFD, STYLESTART FGSILVER STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (strisame(atag, "[=C=Grey]") || strisame(atag, "[=C=Gray]"))
  {
    //White Text
    if (!colourtypespec) updatecolourtypespec();
    if (colourtypespec >= 4) write(SCREENFD, STYLESTART FGGREY STYLEEND, sizeof(char)*(N_STYLESTART+N_FGLIGHT+N_STYLEEND));
    else if (colourtypespec >= 3) write(SCREENFD, STYLESTART FGBLACK STYLEEND, sizeof(char)*(N_STYLESTART+N_FGDARK+N_STYLEEND));
  }
  else if (stristartsas(atag, "[=C=#"))
  {
    //Arbitrary colour text - TODO: need to find out what the terminal can cope with and implement the closest!
    //...
  }
  else if (strisame(atag, "[=C=N]"))
  {
    //Colour Normal - need to get this from settings
  }
  else if (stristartsas(atag, "[=S="))
  {
    //Ignore text size changes - we can't implement this in a terminal!
  }
  else if (strisame(atag, "[Intro]") || stristartsas(atag, "[Intro ") ||
           strisame(atag, "[Ending]") || stristartsas(atag, "[Ending ") ||
           strisame(atag, "[Outro]") || stristartsas(atag, "[Outro ") ||
           strisame(atag, "[Verse]") || stristartsas(atag, "[Verse ") ||
           strisame(atag, "[Chorus]") || stristartsas(atag, "[Chorus ") ||
           strisame(atag, "[Bridge]") || stristartsas(atag, "[Bridge ") ||
           strisame(atag, "[MiddleEight]") || stristartsas(atag, "[MiddleEight ") ||
           strisame(atag, "[Tag]") || stristartsas(atag, "[Tag ") ||
           strisame(atag, "[Slide]") || stristartsas(atag, "[Slide "))
  {
    //Ignore slide definitions
  }
  else
  {
    fprintf("Warning: Unknown tag \"%s\"!\n",atag);
    return 0;
  }
  return 1;
}