#ifndef TERMDISP_INCLUDED
#define TERMDISP_INCLUDED 1

struct rgbcolour
{
  int r;
  ing g;
  int b;
}

union colours_union
{
  struct rgbcolour truecolour;
  int colour16;
};

typedef struct stylesblock_struct
{
  int isbold : 1;
  int isitalics : 1;
  int isundelined : 1;
  int isshadowed : 1;
  int outlinecolour : 2; //0=None, 1=black, 2=white, 3=inverse
  int istruetextcolour : 1;
  int istruebackcolour : 1;
  union colours_union textcolour;
  union colours_union backcolour;
  //Size is ignored
} stylesblock;


void revertcurrenttonormal();
int updatecolourtypespec();
int writestylefromtag(char *atag);


#endif