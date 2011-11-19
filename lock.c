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

typedef struct
{
  OiCapability          capability;
  int             lock;
  pthread_mutex_t mutex;
}  __attribute((packed))  Lock;

static void lock_init (Oi *oi, OiCapability *capability, Oi     *args)
{
  pthread_mutexattr_t attr;
  Lock *lock = (Lock*)capability;
  lock->lock = 0;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

  pthread_mutex_init(&lock->mutex, &attr);
}
static void lock_destroy (Oi *oi, OiCapability *capability)
{
  Lock *lock = (Lock*)capability;
  pthread_mutex_destroy (&lock->mutex);
}
OI(LOCK, Lock, NULL, lock_init, lock_destroy)

#define OI_LOCK(oi) ((Lock*)oi_capability_get_assert (oi, LOCK))

Oi *oi_lock (Oi *oi)
{
  Lock *lock = (Lock*)oi_capability_ensure (oi, LOCK, NULL);
  pthread_mutex_lock (&lock->mutex);
  lock->lock++;
  return oi;
}

int oi_trylock (Oi *oi)
{
  Lock *lock = (Lock*)oi_capability_get (oi, LOCK);
  int res;
  res = pthread_mutex_trylock (&lock->mutex);
  if (!res)
    {
      lock->lock++;
      return 0;
    }
  return res;
}

void  oi_unlock (Oi *oi)
{
  Lock *lock = (Lock*)oi_capability_get (oi, LOCK);
  if (!lock || lock->lock-- == 0)
    fprintf (stderr, "unlocking unlocked lock!\n");
  pthread_mutex_unlock (&lock->mutex);
}
