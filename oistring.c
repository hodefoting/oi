/* Copyright (c) 2011 Øyvind Kolås
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:  Øyvind Kolås    <pippin@gimp.org>
 */

#include "oi.h"
#include "oistring.h"
#include <stdarg.h>
#include <stdio.h>

#define OI_STRING(oi) ((String*)oi_trait_get_assert (oi, STRING))

@trait String
{
  char      *data;
  int         len;
  int   allocated;
};

static void init (Oi *args)
{
  string->allocated = 8;
  string->len = 0;
  string->data = oi_malloc (string->allocated);
}
static void destroy ()
{
  if (string->data)
    oi_free (string->allocated, string->data);
}

void clear ()
{
  String *string = OI_STRING (self);
  string->len = 0;
  string->data[string->len]=0;
}

void appendc (int val)
{
  String *string = OI_STRING (self);
  if (string->len + 2 >= string->allocated)
    {
      char *old = string->data;
      string->allocated *= 2;
      string->data = oi_malloc (string->allocated);
      memcpy (string->data, old, string->allocated/2);
      oi_free (string->allocated/2, old);
    }
  string->data[string->len++] = val;
  string->data[string->len] = '\0';
}

void append_str (const char *str)
{
  while (str && *str)
    {
      self@string:appendc (*str);
      str++;
    }
}

void append_string (Oi *oi2)
{
  const char *str = oi2@string:get ();
  while (str && *str)
    {
      self@string:appendc (*str);
      str++;
    }
}
const char *get ()
{
  String *string = OI_STRING (self);
  return string->data;
}

/* dissolving a string, means destroying it, but returning
 * the string, that should be manually freed.
 */

char *dissolve   ()
{
  String *string = OI_STRING (self);
  char *ret = string->data;
  string->data = NULL;
  self@oi:finalize();
  return ret;
}

void append_printf (const char *format, ...)
{
  va_list ap;
  size_t needed;
  char  *buffer;

  va_start(ap, format);
  needed = vsnprintf(NULL, 0, format, ap) + 1;
  buffer = oi_malloc (needed);
  va_end (ap);

  va_start(ap, format);
  vsnprintf(buffer, needed, format, ap);
  va_end (ap);

  self@string:append_str (buffer);
  oi_free (needed, buffer);
}

@end

Oi *string_new (const char *initial)
{
  Oi *self = oi_new_bare (STRING, NULL);
  if (initial)
    {
      self@string:append_str (initial);
    }
  return self;
}
