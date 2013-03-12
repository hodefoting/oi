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


#include <stdio.h>
#include <pthread.h>
#include "oi.h"

@trait Lock
{
  int             lock;
  pthread_mutex_t mutex;
};

static void init ()
{
  pthread_mutexattr_t attr;
  lock->lock = 0;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&lock->mutex, &attr);
}
static void destroy ()
{
  pthread_mutex_destroy (&lock->mutex);
}
@end

Oi *oi_lock (Oi *self)
{
  Lock *lock = (Lock*)self@oi:capability_ensure (LOCK, NULL);
  pthread_mutex_lock (&lock->mutex);
  lock->lock++;
  return self;
}

int oi_trylock (Oi *self)
{
  Lock *lock = (Lock*)self@oi:capability_get (LOCK);
  int res;
  res = pthread_mutex_trylock (&lock->mutex);
  if (!res)
    {
      lock->lock++;
      return 0;
    }
  return res;
}

void  oi_unlock (Oi *self)
{
  Lock *lock = (Lock*)self@oi:capability_get (LOCK);
  if (!lock || lock->lock-- == 0)
    fprintf (stderr, "unlocking unlocked lock!\n");
  pthread_mutex_unlock (&lock->mutex);
}
