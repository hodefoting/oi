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

typedef struct
{
  OiCapability     capability;
  char      *data;
  int         len;
  int   allocated;
}  __attribute((packed))  String;

static void string_init (Oi *oi, OiCapability *capability, Oi *args)
{
  String *string = (String*)capability;
  string->allocated = 8;
  string->len = 0;
  string->data = oi_malloc (string->allocated);
}
static void string_destroy (Oi *oi, OiCapability *capability)
{
  String *string = (String*)capability;
  if (string->data)
    oi_free (string->allocated, string->data);
}
OI(STRING, String, NULL, string_init, string_destroy)

#define OI_STRING(oi) ((String*)oi_capability_get_assert (oi, STRING))

Oi *oi_string_new (const char *initial)
{
  Oi *oi = oi_new ();
  oi_capability_add (oi, STRING, NULL);
  if (initial)
    oi_string_append_str (oi, initial);
  return oi;
}
void oi_string_clear (Oi *oi)
{
  String *string = OI_STRING (oi);
  string->len = 0;
  string->data[string->len]=0;
}

void oi_string_appendc (Oi *oi, int val)
{
  String *string = OI_STRING (oi);
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
void oi_string_append_str (Oi *oi, const char *str)
{
  while (str && *str)
    {
      oi_string_appendc (oi, *str);
      str++;
    }
}
void oi_string_append_string (Oi *oi, Oi *oi2)
{
  const char *str = oi_string_get (oi2);
  while (str && *str)
    {
      oi_string_appendc (oi, *str);
      str++;
    }
}
const char *oi_string_get (Oi *oi)
{
  String *string = OI_STRING (oi);
  return string->data;
}

/* dissolving a string, means destroying it, but returning
 * the string, that should be manually freed.
 */
char *oi_string_dissolve   (Oi *oi)
{
  String *string = OI_STRING (oi);
  char *ret = string->data;
  string->data = NULL;
  oi_destroy (oi);
  return ret;
}
