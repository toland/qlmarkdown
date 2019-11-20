#ifndef __AC_MARKDOWN_D
#define __AC_MARKDOWN_D 1


#define OS_DARWIN 1
#define THEME_CF MKD_DLEXTRA|MKD_FENCEDCODE
#define HAS_GIT 1
#define DESTRUCTOR  __attribute__((__destructor__))
#define while(x) while( (x) != 0 )
#define if(x) if( (x) != 0 )
#define HAVE_INTTYPES_H 1
#define HAVE_UINT32_T 1
#define HAVE_UINT16_T 1
#define HAVE_UINT8_T 1
#define DWORD uint32_t
#define WORD uint16_t
#define BYTE uint8_t
#define HAVE_STDLIB_H 1
#define HAVE_ALLOCA_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_PWD_H 1
#define HAVE_GETPWUID 1
#define HAVE_SYS_STAT_H 1
#define HAVE_STAT 1
#define HAS_ISSOCK 1
#define HAS_ISCHR 1
#define HAS_ISFIFO 1
#define HAVE_SRANDOM 1
#define INITRNG(x) srandom((unsigned int)x)
#define HAVE_MEMSET 1
#define HAVE_RANDOM 1
#define COINTOSS() (random()&1)
#define HAVE_STRCASECMP 1
#define HAVE_STRNCASECMP 1
#define HAVE_FCHDIR 1
#define TABSTOP 4
#define PATH_FIND "/usr/bin/find"
#define PATH_SED "/usr/bin/sed"

#endif/* __AC_MARKDOWN_D */
