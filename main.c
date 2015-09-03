/*=======================================================================
     winlaunch 
     main.h 
     (c)2012 Kevin Boone
========================================================================*/
#include <windows.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cygwin.h"
#include "defs.h"


/*=======================================================================
     struct Options
     Set up by command-line switches 
========================================================================*/

typedef enum SEMode {SE_UNSPEC = 0, 
  SE_PRINT = 1, SE_EXPLORE = 2, SE_FIND = 3, SE_EDIT = 4, SE_OPEN = 5} SEMode;

struct Options
  {
  BOOL version;
  BOOL debug;
  BOOL help;
  BOOL longhelp;
  SEMode mode; // Mode to pass to ShellExecute
  BOOL force_windows;
  BOOL minimize;
  };


/*=======================================================================
     show_short_usage 
========================================================================*/
void show_short_usage (const char *argv0, FILE *out)
  {
  fprintf (out, "Usage: %s [options] {windows_binary} {arguments}\n", argv0);
  fprintf (out, "'%s --longhelp' for details\n", argv0);
  }


/*=======================================================================
     show_long_usage 
========================================================================*/
void show_long_usage (const char *argv0, FILE *out)
  {
  fprintf (out, "Usage: %s [options] {windows_binary} {arguments}\n", argv0);
  fprintf (out, "  -d, --debug               show debugging information\n"); 
  fprintf (out, "  -e, --explore             open explorer\n"); 
  fprintf (out, "  -f, --find                start a search\n"); 
  fprintf (out, "  -m, --minimize            run minimized\n"); 
  fprintf (out, "  -o, --open                open the file\n"); 
  fprintf (out, "  -p, --print               print the file\n"); 
  fprintf (out, "  -t, --edit                edit the file\n"); 
  fprintf (out, "  -v, --version             show version\n");
  fprintf (out, "  -w, --windows             force use of ShellExecute\n"); 
  }


/*=======================================================================
    process_arg 
========================================================================*/
int process_arg (const char *argv0, const char *arg, struct Options *options)
  {
  if (strlen (arg) < 2) 
    {
    fprintf (stderr, "%s: Invalid command-line switch: '%s'\n", argv0, arg);
    show_short_usage (argv0, stderr);
    return -1;
    }

  if (arg[1] == '-')
    {
    const char *long_arg = arg + 2;
    if (strlen (long_arg) < 1) 
      {
      fprintf (stderr, "%s: Invalid command-line switch: '%s'\n", argv0, arg);
      show_short_usage (argv0, stderr);
      return -1;
      } 
    if (strcmp (long_arg, "debug") == 0)
      options->debug = TRUE;
    else if (strcmp (long_arg, "version") == 0)
      options->version = TRUE;
    else if (strcmp (long_arg, "help") == 0)
      options->help = TRUE;
    else if (strcmp (long_arg, "longhelp") == 0)
      options->longhelp = TRUE;
    else if (strcmp (long_arg, "print") == 0)
      options->mode = SE_PRINT;
    else if (strcmp (long_arg, "explore") == 0)
      options->mode = SE_EXPLORE;
    else if (strcmp (long_arg, "find") == 0)
      options->mode = SE_FIND;
    else if (strcmp (long_arg, "edit") == 0)
      options->mode = SE_EDIT;
    else if (strcmp (long_arg, "open") == 0)
      options->mode = SE_OPEN;
    else if (strcmp (long_arg, "windows") == 0)
      options->force_windows = TRUE;
    else if (strcmp (long_arg, "minimize") == 0)
      options->minimize = TRUE;

    }
  else
    {
    int i, l = strlen (arg);
    for (i = 1; i < l; i++)
      {
      switch (arg[i])
        {
        case 'h':
          options->help = TRUE;
          break;
        case 'v':
          options->version = TRUE;
          break;
        case 'd':
          options->debug = TRUE;
          break;
        case 'p':
          options->mode = SE_PRINT;
          break;
        case 'e':
          options->mode = SE_EXPLORE;
          break;
        case 'f':
          options->mode = SE_FIND;
          break;
        case 't':
          options->mode = SE_EDIT;
          break;
        case 'o':
          options->mode = SE_OPEN;
          break;
        case 'w':
          options->force_windows = TRUE;
          break;
        case 'm':
          options->minimize = TRUE;
          break;
        default:
          fprintf (stderr, "%s: Invalid command-line switch: '%c'\n", 
            argv0, arg[i]);
          show_short_usage (argv0, stderr);
          return -1;
          break;
        }
      }
    }

  return 0;
  }

/*=======================================================================
     main 
========================================================================*/
int main (int argc, char **argv)
  {
  BOOL debug = FALSE;

  if (argc < 2)
    {
    show_short_usage(argv[0], stderr);
    return -1;
    }

  // Process switches and find first non-switch argument
  struct Options options;
  memset (&options, 0, sizeof (options));
  int i, first_nonswitch = 0;
  for (i = 1; i < argc; i++)
    {
    const char *arg = argv[i];
    if (strlen (arg) == 0) break;
    if (arg[0] == '-')
      {
      int r;
      if ((r = process_arg (argv[0], arg, &options)) != 0) 
        return r;
      }
    else
      break;
    }


  // Process arguments that prevent further operation

  if (options.version)
    {
    printf ("winlaunch version " VERSION ":\n"
     "Copyright (c)2012 Kevin Boone.\n"
     "Distributed according to the terms of the "
     "GNU Public Licence, version 2.\n");
    return 0;
    }

  if (options.help)
    {
    show_short_usage (argv[0], stdout);
    return 0;
    }

  if (options.longhelp)
    {
    show_long_usage (argv[0], stdout);
    return 0;
    }

  // Switches OK -- check there is at least one argument left

  first_nonswitch = i;
  if (first_nonswitch >= argc)
    {
    fprintf (stderr, "%s: Not enough arguments.\n", argv[0]);
    show_short_usage (argv[0], stderr);
    return -1;
    }

  if (options.debug) debug = TRUE;

  char *windows_binary = argv[first_nonswitch];

  // Bomb is the first non-switch argument is not a file that can
  //  be inspected
  struct stat sb;
  if (stat (windows_binary, &sb) != 0)
    {
    fprintf (stderr, "%s: %s: %s\n", argv[0], windows_binary, 
      strerror(errno));
    return errno;
    }

  BOOL is_dir;
  if (S_ISDIR (sb.st_mode))
    is_dir = TRUE;
  else
    is_dir = FALSE;
 
  BOOL force_winexec = FALSE; 
  if (options.mode == SE_PRINT || options.mode == SE_EXPLORE)
    force_winexec = TRUE;
  if (options.force_windows == TRUE)
    force_winexec = TRUE;

  // Construct new array of strings for arguments to pass to exec,
  //  starting with the first non-switch argument

  char **new_args = (char **)malloc ((argc + 2) * sizeof (char *));
  int j = 0;
  for (i = first_nonswitch; i < argc; i++)
    {
    char *new_arg;
    if (strlen(argv[i]) == 0)
      {
      new_arg = strdup ("");
      }
    else if (argv[i][0] == '-')
      {
      new_arg = strdup (argv[i]);
      }
    else if (strstr (argv[i], "//") != 0)
      {
      // Arg containing a // is probably a URL. Do not convert
      new_arg = strdup (argv[i]);
      }
    else
      {
      // Note -- path_to_windows will erroneously convert URLs
      new_arg = cygwin_path_to_windows (argv[i]);
      }
    if (debug)
      {
      printf ("arg %d = %s\n", j, new_arg);
      }
    new_args[j++] = new_arg;
    } 

  new_args[j] = NULL;
  int new_argc = j;

  // Either process using ShellExecute or execv, depending on whether file
  //  is executable or not
  SHFILEINFO sfi;
  int r = (int)SHGetFileInfo (new_args[0], 0, &sfi, sizeof(sfi), SHGFI_EXETYPE);
  if (r == 0 || force_winexec)
    {
    if (debug)
      printf ("Using ShellExecute API\n");

    char *operation = NULL;

    switch (options.mode)
      {
      case SE_EXPLORE:
        operation = "explore";
        break;
      case SE_PRINT:
        operation = "print";
        break;
      case SE_FIND:
        operation = "find";
        break;
      case SE_EDIT:
        operation = "edit";
        break;
      case SE_OPEN:
        operation = "open";
        break;
      default:;
      }

    if (is_dir && !operation)
      operation = "explore";

   if (operation == NULL) operation = "open";

   if (debug)
      printf ("SE operation is '%s'\n", operation);

    // This is not an executable file, or the user has forced the use of the
    // Windows API. Try passing it to ShellExec as a document

    // Build a single string out of command line arguments
    char *params = NULL;
    if (new_argc > 0)
      {
      int param_len = 0;
      for (j = 1; j < new_argc; j++) 
        {
        param_len  += strlen (new_args[j]) + 5; // space for quotes, etc
        }

      params = (char *)malloc (param_len * sizeof(char) + 1);
      params[0] = 0;
      for (j = 1; j < new_argc; j++) 
        {
        strcat (params, " ");
        strcat (params, "\"");
        strcat (params, new_args[j]);
        strcat (params, "\"");
        }

      if (debug)
        printf ("Windows parameters are: '%s'\n", params);
      }
    
    int r = (int)ShellExecute (NULL, operation, new_args[0], 
      params, NULL, options.minimize ? SW_MINIMIZE : SW_SHOW);

    if (params)
      free (params);

    if (r != 0 && r != 42)
      {
      // For some reason, executing a directory succeeds, but still returns
      //  error 42 (an undefined return value). We don't want to 
      //  report this.
      switch (r)
        {
        case SE_ERR_ACCESSDENIED:
          fprintf (stderr, "%s: Access denied to %s (may be bogus)\n", 
            argv[0], windows_binary);
          break;
        case SE_ERR_NOASSOC:
          fprintf (stderr, "%s: No association defined for %s\n", 
            argv[0], windows_binary);
          break;
        default:
          fprintf (stderr, "%s: Windows error %d launching %s\n", 
            argv[0], r, windows_binary);
        }
      }
    return 0;
    }
  else
    {
    execvp (windows_binary, new_args);

    // We should never get here. If we do, execvp() has failed.
  
    fprintf (stderr, "%s: could not launch %s: %s\n", argv[0], windows_binary, 
      strerror(errno));
    return errno;
    }
  }



