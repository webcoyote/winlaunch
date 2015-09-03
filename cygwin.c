/*=======================================================================
     winlaunch 
     cygwin.c
     (c)2012 Kevin Boone
     Any specific cygwin API code goes in here
========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <sys/cygwin.h>
#include <errno.h>


/*=======================================================================
     cygwin_path_to_windows
     Converts a unix path to native windows format, reversing directory
     separators, etc. The caller must free the string returned.
========================================================================*/
char *cygwin_path_to_windows (const char *unix_path)
  {
  char *result = malloc (PATH_MAX);
#ifdef WIN64
  cygwin_conv_path (CCP_POSIX_TO_WIN_A, unix_path, result, PATH_MAX -1 );
#else
  cygwin_conv_to_win32_path (unix_path, result);
#endif
  return result;
  }

