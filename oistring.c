/* Copyright (c) 2011,2013 Øyvind Kolås
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
@generateheader
#include <stdarg.h>
#include <stdio.h>

#define OI_STRING(oi) ((String*)trait_get_assert (oi, STRING))

@trait String
{
  char      *data;
  int         len;
  int   allocated;
};

/* this is where the initial string should be set.. */
static void init (const char *initial)
{
  this->allocated = 8;
  this->len = 0;
  this->data = oi_malloc (this->allocated);

  if (initial)
    {
      self@string:append_str (initial);
    }
}
static void destroy ()
{
  if (this->data)
    oi_free (this->allocated, this->data);
}

var clear ()
{
  String *this = OI_STRING (self);
  this->len = 0;
  this->data[this->len]=0;
  return self;
}

var appendc (int val)
{
  String *this = OI_STRING (self);
  if (this->len + 2 >= this->allocated)
    {
      char *old = this->data;
      this->allocated *= 2;
      this->data = oi_malloc (this->allocated);
      memcpy (this->data, old, this->allocated/2);
      oi_free (this->allocated/2, old);
    }
  this->data[this->len++] = val;
  this->data[this->len] = '\0';
  return self;
}

var append_str (const char *str)
{
  while (str && *str)
    {
      self = self@string:appendc (*str);
      str++;
    }
  return self;
}

var append_string (var oi2)
{
  const char *str = oi2@string:get ();
  while (str && *str)
    {
      self = self@string:appendc (*str);
      str++;
    }
  return self;
}
const char *get ()
{
  String *this = OI_STRING (self);
  return this->data;
}

/* dissolving a string, means destroying it, but returning
 * the string, that should be manually freed.
 */

char *dissolve   ()
{
  String *this = OI_STRING (self);
  char *ret = this->data;
  this->data = NULL;
  self@var:finalize();
  return ret;
}

var append_printf (const char *format, ...)
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

  self = self@string:append_str (buffer);
  oi_free (needed, buffer);
  return self;
}

var new (const char *initial)
{
  return var_new (STRING, (void*)initial);
}

@end
