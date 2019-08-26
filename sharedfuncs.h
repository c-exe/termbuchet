#ifndef SHAREDFUNCS_INCLUDED
#define SHAREDFUNCS_INCLUDED 1

#define SCREENFD 1
#define STDINFD 0
#define STDERRFD 2

#define MSG_SEPARATOR "\r\n"
#define MSG_IS "="
#define MSG_RECEIVED "ROK"
#define MSG_EXECUTED "XOK"
#define MSG_COLOURTYPE "COLOUR"
#define MSG_CT_MONOCHROME "1"
#define MSG_CT_EIGHT "3"
#define MSG_CT_SIXTEEN "4"
#define MSG_CT_BYTE "8"
#define MSG_CT_TRUECOLOUR "24"
#define MSG_WIDTH "WIDTH"
#define MSG_HEIGHT "HEIGHT"
#define MSG_METAFORMAT "METAFORMAT"
#define MSG_CMD "DO_"
#define MSG_CMD_CLS "CLS"
#define MSG_CMD_SLIDE "SLIDE"
#define MSG_CMD_SLIDEMETA "SLIDEMETA"
#define MSG_REQUEST "RQ_"

#define MSG_SLIDEHEADER_START MSG_CMD MSG_CMD_SLIDE " LENGTH" MSG_IS /* LENGTH */
#define MSG_SLIDEHEADER_END "\n"
#define MSG_SLIDEFOOTER "\033" MSG_CMD_SLIDE

#define MSG_SLIDEMETAHEADER_START MSG_CMD MSG_CMD_SLIDEMETA " LENGTH" MSG_IS /* LENGTH */
#define MSG_SLIDEMETAHEADER_END "\n"
#define MSG_SLIDEMETAFOOTER "\033" MSG_CMD_SLIDEMETA

int strsame(char *str1, char *str2);
int strisame(char *str1, char *str2);
int strstartsas(char *str, char *start);
int stristartsas(char *str, char *start);


#endif