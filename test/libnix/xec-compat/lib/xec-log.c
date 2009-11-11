/*
 * XEC - Optimizing Dynarec Engine
 *
 * Logging routines
 * Copyright (C) 2007 Orlando Bassotto. All rights reserved.
 * Copyright (C) 2007 Marcello Barnaba. All rights reserved.
 * Copyright (C) 2007 Gianluca Guida. All rights reserved.
 * 
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "xec-debug.h"
#include "xec-mem.h"

typedef struct _log_tag
  {
    TAILQ_ENTRY (_log_tag)  link;
    char                   *name;
    void                   *cookie;
    xec_log_severity_t      min;
    unsigned                flags;
    FILE                   *output;
#define LOG_ENABLED 1
  } log_tag_t;

typedef TAILQ_HEAD (_log_tagq, _log_tag) log_tagq_t;

static unsigned   g_log_flags = XEC_LOG_TIMESTAMP | XEC_LOG_SOURCEINFO;

static size_t     g_ntags = 0;
static log_tag_t  g_global;
static log_tagq_t g_tags;

static __inline log_tag_t *
log_find_tag (char const *tag)
{
  log_tag_t *ltag;

  if (tag == NULL)
    return &g_global;

  TAILQ_FOREACH (ltag, &g_tags, link)
    {
      if (strcmp (tag, ltag->name) == 0)
        return ltag;
    }

  return NULL;
}

static void
log_timestamp (FILE *fp)
{
  char           timestamp[32];
  struct timeval tv;

  gettimeofday (&tv, NULL);

  strftime (timestamp, sizeof (timestamp), "%Y-%m-%d %H:%M:%S",
    localtime ( (const time_t *)&tv.tv_sec));

  fprintf (fp, "[%s.%03u]", timestamp, tv.tv_usec / 1000);
}

static void
log_close (log_tag_t *ltag)
{
  if (ltag->output != NULL && ltag->output != stdout && ltag->output != stderr)
    {
      log_timestamp (ltag->output);
      fprintf (ltag->output, " LOG CLOSED\n");

      fflush (ltag->output);
      fclose (ltag->output);
      ltag->output = stderr;
    }
}

static void
log_close_all (void)
{
  log_tag_t *ltag;

  TAILQ_FOREACH (ltag, &g_tags, link)
    {
      log_close (ltag);
    }
}

void
xec_log_cleanup (void)
{
  log_close_all ();
}

uint32_t
xec_log_get_flags (void)
{
  return g_log_flags;
}

void
xec_log_set_flags (uint32_t flags)
{
  g_log_flags = flags;
}

void
xec_abort (bool coredump)
{
  log_close_all ();

  /* Disable signal handlers. */
  sigsetmask(0);
  signal(SIGSEGV, SIG_DFL);
#ifdef SIGBUS
  signal(SIGBUS, SIG_DFL);
#endif

  if (coredump)
    *(int *)(-1) = 1;
  else
    abort ();
}

void
__xec_log_init (void)
{
  g_global.name   = "<global>";
  g_global.min    = XEC_LOG_DEBUG;
  g_global.cookie = &g_global;
  g_global.flags  = LOG_ENABLED;
  g_global.output = stderr;
  TAILQ_INIT (&g_tags);
}

void *
xec_log_register (char const *tag)
{
  log_tag_t *ltag;

  ltag = log_find_tag (tag);
  if (ltag != NULL)
    return ltag->cookie;

  ltag = xec_mem_alloc_type (log_tag_t, 0);
  if (ltag == NULL)
    return NULL;

  ltag->name   = xec_mem_strdup (tag);
  ltag->cookie = ltag;
  ltag->min    = XEC_LOG_ERROR;
  ltag->flags  = LOG_ENABLED;
  ltag->output = stderr;
  TAILQ_INSERT_TAIL (&g_tags, ltag, link);
  g_ntags++;

  return ltag->cookie;
}

void
xec_log_unregister (void *cookie)
{
  log_tag_t *tag;
  
  if (cookie == NULL || cookie == g_global.cookie)
    return;

  tag = (log_tag_t *)cookie;
  TAILQ_REMOVE (&g_tags, tag, link);

  log_close (tag);

  xec_mem_free (tag->name);
  xec_mem_freef (tag, XEC_MEM_ZERO);
}

bool
xec_log_enable (char const *tag)
{
  log_tag_t *ltag = log_find_tag (tag);

  if (ltag == NULL)
    return false;

  ltag->flags |= LOG_ENABLED;
  if (ltag->output == NULL)
    ltag->output = stderr;

  return true;
}

bool
xec_log_disable (char const *tag)
{
  log_tag_t *ltag = log_find_tag (tag);

  if (ltag == NULL)
    return false;

  ltag->flags &= ~LOG_ENABLED;
  return true;
}

void
xec_log_disable_all (void)
{
  log_tag_t *ltag;
  TAILQ_FOREACH (ltag, &g_tags, link)
    {
      ltag->flags &= ~LOG_ENABLED;
    }
}

bool
xec_log_is_enabled (char const *tag)
{
  log_tag_t *ltag = log_find_tag (tag);

  if (ltag == NULL)
    return false;

  return (ltag->flags & LOG_ENABLED) != 0;
}

bool
xec_log_token_is_enabled (void *p)
{
  log_tag_t *ltag = (log_tag_t *)p;

  if (ltag == NULL)
    return false;

  return (ltag->flags & LOG_ENABLED) != 0;
}

void
xec_log_set_min_severity (char const         *tag,
                          xec_log_severity_t  severity)
{
  log_tag_t *ltag = log_find_tag (tag);

  if (ltag == NULL)
    return;

  if (severity > XEC_LOG_ERROR)
    severity = XEC_LOG_ERROR;

  ltag->min = severity;
}
  
xec_log_severity_t
xec_log_get_min_severity (char const *tag)
{
  log_tag_t *ltag = log_find_tag (tag);

  if (ltag != NULL)
    return ltag->min;
  else
    return XEC_LOG_FATAL;
}

void
xec_log_set_file (char const *tag, FILE *f)
{
  log_tag_t *ltag;

  ltag = log_find_tag (tag);
  if (ltag == NULL)
    return;

  ltag->output = f;
}

FILE *
xec_log_token_get_file (void *p)
{
  log_tag_t *ltag = (log_tag_t *)p;
  if (ltag != NULL)
    return ltag->output;
  return NULL;
}

FILE *
xec_log_get_file (char const *tag)
{
  log_tag_t *ltag;

  ltag = log_find_tag (tag);
  if (ltag != NULL)
    return ltag->output;
  else
    return NULL;
}

bool
xec_log_to_file (char const *tag, char const *name)
{
  log_tag_t *ltag;
  FILE      *f;
  char      *filename = NULL;

  ltag = log_find_tag (tag);
  if (ltag == NULL)
    return false;

  if (name == NULL)
    {
      char *buf = xec_mem_alloc (strlen (tag) + 5, XEC_MEM_ZERO);
      if (buf != NULL)
        {
          sprintf (buf, "%s.log", tag);
          filename = buf;
        }
    }

  if (filename == NULL)
    filename = xec_mem_strdup (name);

  if (filename == NULL) // not every asprintf will set this pointer
    return false;       // to NULL in case of failure, but we have to
                        // be *brutal* ;)

  f = fopen (filename, "a+");

  xec_mem_free (filename);

  if (f == NULL)
    {
      XEC_LOG (NULL, XEC_LOG_FATAL, 0, "Cannot open %s logfile: %s",
          tag, strerror (errno));
      return false;
    }
  else
    {
      log_timestamp (f);
      fprintf (f, " LOG OPENED\n");

      ltag->output = f;
      return true;
    }
}

bool
xec_log_to_stderr (char const *tag)
{
  log_tag_t *ltag;
  ltag = log_find_tag (tag);

  if (ltag == NULL)
    return false;

  if (ltag->output == stderr)
    return false;

  log_close (ltag);
  return true;
}

/* Example string: +foo,-bar,*baz,*prot:prot.log .. where
 *
 * - foo, bar and baz are log tokens
 * - '-' means don't log
 * - '+' means log to stderr
 * - '=' means log to file (changed from '*' because may confuse the shell)
 * - prot.log is the log file name
 */
void
xec_log_parse_options (char const *optstring)
{
  char *s, *tag, *end, *logname;
  int mode;

  for (s = xec_mem_strdup (optstring), tag = s; tag != NULL; tag = end)
    {
      bool rc = false;

      end = strchr (tag, ',');
      if (end != NULL)
        *end++ = '\0';

      switch (*tag)
        {
        case '+':
        case '-':
        case '=':
          mode = *tag++;
          break;

        default:
          mode = '+'; // log to stderr by default
          break;
        }

      logname = strchr (tag, ':');
      if (logname != NULL)
        *logname++ = '\0';

      switch (mode)
        {
        case '+':
          rc = xec_log_enable (tag);
          break;
        case '-':
          rc = xec_log_disable (tag);
          break;
        case '=':
          rc = xec_log_to_file (tag, logname);
          if (rc)
            xec_log_enable (tag);
          break;
        default:
          break;
        }

      if (!rc)
        {
          XEC_LOG (NULL,
                   XEC_LOG_WARNING,
                   0,
                   "cannot find log tag `%s'",
                   tag);
        }
    }

  xec_mem_free (s);
}

void
xec_log (void               *cookie,
         xec_log_severity_t  severity,
         unsigned            flags,
         char const         *function,
         char const         *filename,
         unsigned            line,
         char const         *format,
         ...)
{
  va_list ap;

  va_start (ap, format);
  xec_logv (cookie, severity, flags, function, filename, line, format, ap);
  va_end (ap);
}

void
xec_logv (void               *cookie,
          xec_log_severity_t  severity,
          unsigned            flags,
          char const         *function,
          char const         *filename,
          unsigned            line,
          char const         *format,
          va_list             ap)
{
  bool            core;
  bool            enabled;
  log_tag_t      *tag;
  char const     *fmtend;
  char           *fmt = NULL;
  
  tag = (log_tag_t *)cookie;
  if (tag == NULL)
    tag = &g_global;

  core = (severity == XEC_LOG_FATAL || (flags & XEC_LOG_CORE) != 0);
  enabled = core || (tag != NULL && (tag->flags & LOG_ENABLED) != 0 && (tag->min <= severity));

  if (!enabled || format == NULL)
    return;

  fmtend = format + strlen (format) - 1;
  if (*fmtend == '\r' || *fmtend == '\n')
    {
      while (fmtend != format && (*fmtend == '\r' || *fmtend == '\n'))
        --fmtend;

      if (fmtend == format)
        return;

      fmt = xec_mem_strndup (format, fmtend - format + 1);
    }

  if (fmt == NULL)
    fmt = (char *)format;

  if (tag->output == stderr)
    {
      fflush (stdout);
      fflush (stderr);
    }

  if (flags & XEC_LOG_NEWLINE)
    fprintf (tag->output, "\n");

  if (g_log_flags & XEC_LOG_TIMESTAMP)
    log_timestamp (tag->output);

  if (g_log_flags & XEC_LOG_SOURCEINFO)
    {
      if (filename != NULL && line != 0) {
        if (strlen(filename) > 20)
          fprintf (tag->output, "[...%s:%u]", filename + strlen (filename) - 20, line);
        else
          fprintf (tag->output, "[%s:%u]", filename, line);
      }
      fprintf (tag->output, " (%u): ", getpid ());
      if (function != NULL)
        fprintf (tag->output, "%s: ", function);
    }

  fprintf (tag->output, "[%s] ", tag->name);
  if (!(g_log_flags & XEC_LOG_SOURCEINFO))
    {
      if (function != NULL)
        fprintf (tag->output, "%s: ", function);
    }

  switch (severity)
    {
    case XEC_LOG_FATAL:   fprintf (tag->output, "FATAL: "); break;
    case XEC_LOG_ERROR:   fprintf (tag->output, "ERROR: "); break;
    case XEC_LOG_WARNING: fprintf (tag->output, "WARNING: "); break;
    case XEC_LOG_INFO:    fprintf (tag->output, "INFO: "); break;
    case XEC_LOG_DEBUG:   fprintf (tag->output, "DEBUG: "); break;
    }

  vfprintf (tag->output, fmt, ap);

  fprintf (tag->output, "\n");

  if (tag->output == stderr)
    fflush (stderr);

  if (fmt != (char *)format)
    xec_mem_free (fmt);

  if ( (core || severity == XEC_LOG_FATAL) && (flags & XEC_LOG_DONTEXIT) == 0)
    {
      log_close_all ();

      if (flags & XEC_LOG_ERREXIT)
        exit (EXIT_FAILURE);
      else if (flags & XEC_LOG_CLEANEXIT)
        exit (EXIT_SUCCESS);
      else
        xec_abort (core);
    }
}

size_t
xec_log_get_tags (char const **tags)
{
  log_tag_t *tag;
  size_t     n = 0;

  if (tags == NULL)
    return g_ntags;

  TAILQ_FOREACH (tag, &g_tags, link)
    {
      tags[n++] = tag->name;
    }

  return g_ntags;
}
