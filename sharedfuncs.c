#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctypes.h>

int strsame(char *str1, char *str2)
{
  long i;
  for (i=0; str1[i]!=0 && str2[i]!=0; i++)
  {
    if (str1[i] != str2[i]) return 0;
  }
  if (str1[i] != str2[i]) return 0;
  return 1;
}

int strisame(char *str1, char *str2)
{
  long i;
  char a1, a2;
  for (i=0; str1[i]!=0 && str2[i]!=0; i++)
  {
    a1=toupper(str1[i]);
    a2=toupper(str2[i]);
    if (a1 != a2) return 0;
  }
  if (str1[i] != str2[i]) return 0;
  return 1;
}

int strstartsas(char *str, char *start)
{
  long i;
  for (i=0; str[i]!=0 && start[i]!=0; i++)
  {
    if (str[i] != start[i]) return 0;
  }
  if (str[i]==0 && start[i]!=0) return 0;
  return 1;
}

int stristartsas(char *str, char *start)
{
  long i;
  for (i=0; str[i]!=0 && start[i]!=0; i++)
  {
    a1=toupper(str[i]);
    a2=toupper(start[i]);
    if (a1 != a2) return 0;
  }
  if (str[i]==0 && start[i]!=0) return 0;
  return 1;
}
