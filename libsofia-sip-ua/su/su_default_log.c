/*
 * This file is part of the Sofia-SIP package
 *
 * Copyright (C) 2005 Nokia Corporation.
 *
 * Contact: Pekka Pessi <pekka.pessi@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

/**@ingroup su_log
 * @CFILE su_default_log.c
 *
 * Default debug log object.
 *
 * @author Pekka Pessi <Pekka.Pessi@nokia.com>
 *
 * @date Created: Fri Feb 23 17:30:46 2001 ppessi
 */

#include <stdio.h>
#include <stdarg.h>

#include <sofia-sip/su_log.h>
#include <sofia-sip/su_debug.h>

/** Log into FILE, by default stderr. */
static void default_logger(
      void *stream,
      unsigned level,
      char const *file,
      unsigned line,
      char const *fmt,
      va_list ap)
{
  char buf[1024];
  char *p = buf;
  int l=0;
  int r=1024;
  FILE *f = stream ? (FILE *)stream : stderr;

  if (file != NULL && file[0] != '\0')
  {
     l = snprintf(buf, 1024, "%s:%u: ", file, line);
     if (l > 0)
     {
        if (l<1024)
        {
           --l; // remove trailing '\0'
           p += l;
           r = 1024 - l;
           vsnprintf(p, r, fmt, ap);
           fprintf(f, "%s", buf);
        }
        else
        {
           // truncated.
           // TODO -- filename + line_num exceeded 1023 chars??
           // no-op for now
        }
     }
     else
     {
        // TODO -- string printing error of some sort
        // no-op for now
     }
  }
  else
  {
     vfprintf(f, fmt, ap);
  }
}

/**@var SOFIA_DEBUG
 *
 * Environment variable determining the default debug log level.
 *
 * The SOFIA_DEBUG environment variable is used to determine the default
 * debug logging level. The normal level is 3.
 *
 * @sa <sofia-sip/su_debug.h>, su_log_global
 */
extern char const SOFIA_DEBUG[];

#ifdef SU_DEBUG
#define SOFIA_DEBUG_ SU_DEBUG
#else
#define SOFIA_DEBUG_ 3
#endif

/**Default debug log.
 *
 * If a source module does not define a log object, the output from su_log()
 * function or SU_DEBUG_X() macros use this log object. Also, if a log
 * function references log object with NULL pointer, the su_log_default
 * object is used.
 *
 * If output from another log object is not redirected with
 * su_log_redirect(), the output can be redirected via this log object.
 *
 * If the logging level of a log object is not set with su_log_set_level(),
 * or the environment variable directing its level is not set, the log level
 * from the #su_log_default object is used.
 *
 * The level of #su_log_default is set using SOFIA_DEBUG environment
 * variable.
 */
su_log_t su_log_default[1] = {{
  sizeof(su_log_t),    /* log_size */
  "sofia",             /* log_name */
      /* Environment variable controlling logging level */
  "SOFIA_DEBUG",       /* log_env */
      /* Default level */
  SOFIA_DEBUG_,        /* log_default */
      /* Maximum log level */
  SU_LOG_MAX,          /* log_level */
  "",                  /* log_file */
  0,                   /* log_line */
  0,                   /* log_init */
  default_logger,      /* log_logger */
  NULL                 /* log_stream */
}};
