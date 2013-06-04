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
@generateheader

@trait Ref
{
  int count;
};

static void init ()
{
  ref->count = 1;
}

Oi *inc ()
{
  Ref *ref = self@oi:trait_ensure (REF, NULL);
  if (self@oi:trait_get (MUTEX))
    self@mutex:lock ();
  ref->count++;
  if (self@oi:trait_get (MUTEX))
    self@mutex:unlock ();
  return self;
}

Oi *dec ()
{
  Ref *ref = self@oi:trait_get (REF);
  if (self@oi:trait_get (MUTEX))
    self@mutex:lock ();
  if (!ref || -- ref->count == 0)
    {
      self@oi:finalize ();
      return NULL;
    }
  else
    if (self@oi:trait_get (MUTEX))
      self@mutex:unlock ();
  return self;
}

@end
