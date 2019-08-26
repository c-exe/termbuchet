#ifndef SHAREDFUNCS_INCLUDED
#define SHAREDFUNCS_INCLUDED 1

#define SCREENFD 1
#define STDINFD 0
#define STDERRFD 2

int strsame(char *str1, char *str2);
int strisame(char *str1, char *str2);
int strstartsas(char *str, char *start);
int stristartsas(char *str, char *start);


#endif