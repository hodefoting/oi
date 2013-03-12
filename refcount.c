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

@trait Refcount
{
  int refcount;
};

static void init ()
{
  refcount->refcount = 1;
}

Oi *inc ()
{
  Refcount *refcount = (Refcount*)self@oi:capability_ensure (REFCOUNT, NULL);
  if (self@oi:capability_get (LOCK))
    self@oi:lock ();
  refcount->refcount++;
  if (self@oi:capability_get (LOCK))
    self@oi:unlock ();
  return self;
}

void dec ()
{
  Refcount *refcount = (Refcount*)self@oi:capability_get (REFCOUNT);
  if (self@oi:capability_get (LOCK))
    self@oi:lock ();
  if (!refcount || -- refcount->refcount == 0)
    self@oi:finalize ();
  else
    if (self@oi:capability_get (LOCK))
      self@oi:unlock ();
}

@end

Oi *oi_ref (Oi *self)
{
  Refcount *refcount = (Refcount*)self@oi:capability_ensure (REFCOUNT, NULL);
  if (self@oi:capability_get (LOCK))
    self@oi:lock ();
  refcount->refcount++;
  if (self@oi:capability_get (LOCK))
    self@oi:unlock ();
  return self;
}

void  oi_unref (Oi *self)
{
  Refcount *refcount = (Refcount*)self@oi:capability_get (REFCOUNT);
  if (self@oi:capability_get (LOCK))
    self@oi:lock ();
  if (!refcount || -- refcount->refcount == 0)
    self@oi:finalize ();
  else
    if (self@oi:capability_get (LOCK))
      self@oi:unlock ();
}
