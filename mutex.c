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


#include <stdio.h>
#include <pthread.h>
#include "oi.h"
@generateheader

@trait Mutex
{
  pthread_mutex_t mutex;
  char            lock;
} PACKED;

static void init ()
{
  pthread_mutexattr_t attr;
  this->lock = 0;
  pthread_mutexattr_init(&attr);
//  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_DEFAULT);
  pthread_mutex_init(&this->mutex, &attr);
}
static void destroy ()
{
  pthread_mutex_destroy (&this->mutex);
}

var lock ()
{
  pthread_mutex_lock (&this->mutex);
  this->lock++;
  return self;
}

int trylock ()
{
  int res;
  res = pthread_mutex_trylock (&this->mutex);
  if (!res)
    {
      this->lock++;
      return 0;
    }
  return res;
}

void unlock ()
{
  if (!this || this->lock-- == 0)
    fprintf (stderr, "unlocking unlocked mutex!\n");
  pthread_mutex_unlock (&this->mutex);
}

static void dup (){}

@end
