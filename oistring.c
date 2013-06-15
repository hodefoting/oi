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

@trait String
{
  char      *data;
  int         len;
  int   allocated;
} PACKED;

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
  this->len = 0;
  this->data[this->len]=0;
  return self;
}

char *unicode_to_utf8! (char *p, unsigned int code)
{
  /* attribution:
   * http://stackoverflow.com/questions/4607413/c-library-to-convert-unicode-code-points-to-utf8
   * answer by http://stackoverflow.com/users/379897/r*/
  char *orig_p = p;
  if (code<0x80)
    *p++=code;
  else if (code<0x800)
    {
      *p++=192+code/64;
      *p++=128+code%64;
    }
  else if (code-0xd800u<0x800)
    goto error;
  else if (code<0x10000)
    {
      *p++=224+code/4096;
      *p++=128+code/64%64;
      *p++=128+code%64;
    }
  else if (code<0x110000)
    {
      *p++=240+code/262144;
      *p++=128+code/4096%64;
      *p++=128+code/64%64;
      *p++=128+code%64;
    }
  else
    goto error;
  *p = '\0';
  return p;
error:
  *orig_p = '\0';
  return orig_p;
}

/* given that this is first byte of the character,
 i ow many bytes is the character occupy?
*/
int utf8_len !(unsigned char ch)
{
if (ch < 0x80u) return 1;
else if (ch < 0xE0u) return 2;
else if (ch < 0xF0u) return 3;
else if (ch < 0xF8u) return 4;
else if (ch < 0xFCu) return 5;
else return 6;
}

/** from nchant by pippin: */

int utf8_strlen (unsigned char *s)
{
  int count;
  if (!s)
    return 0;
  for (count = 0; *s; s++)
    if ((*s & 0xC0) != 0x80)
      count ++;
  return count;
}
 
/* given that this is first byte of the character,
   what is the code value of that character?

attribution: http://mytecblog.wordpress.com/2008/12/23/simple-utf-8-c-decoder/
   */
unsigned int utf8_to_unicode! (const char* ch)
{
  unsigned int value = 0;
  int len = utf8_len (*ch);
  switch (len)
  {
    case 6:
      value = ch[0] & 0x01;
      break;
    case 5:
      value = ch[0] & 0x03;
      break;
    case 4:
      value = ch[0] & 0x07;
      break;
    case 3:
      value = ch[0] & 0x0F;
      break;
    case 2:
      value = ch[0] & 0x1F;
      break;
    case 1:
      value = ch[0];
      break;
  }
  for ( int i= 1; i < len; i++ )
    value = value << 6 | (ch[i] & 0x3F);
  return value;
}

var appendc (int val)
{
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

var append_unicode (unsigned int val)
{
  char buf[6];
  unicode_to_utf8 (buf, val);
  return (self@string:append_str (buf));
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
  return this->data;
}

/* dissolving a string, means destroying it, but returning
 * the string, that should be manually freed.
 */

char *dissolve   ()
{
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
