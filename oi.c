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
#include <stdlib.h>
#include "oi.h"

@trait Oi
{
  int             capability_count;
  /* XXX: this could be a treap */
  OiCapability  **capabilities;
};

int
capability_check (OiType *capability)
{
  int i;
  if (self->capability_count == -66) fprintf (stderr, "Eeek");
  if (capability == OI)
    return 1;
  for (i = 0; i < self->capability_count; i++)
    if (self->capabilities[i]->type == capability)
      return 1;
  return 0;
}

void *capability_get (OiType *capability)
{
  int i;
  if (self->capability_count == -66) fprintf (stderr, "Eeek");
  if (capability == OI)
    return (OiCapability*)self;
  for (i = 0; i < self->capability_count; i++)
    if (self->capabilities[i]->type == capability)
      return self->capabilities[i];
  return NULL;
}

void *capability_get_assert (OiType *capability)
{
  OiCapability *res = self@oi:capability_get (capability);
  if (self->capability_count == -66) fprintf (stderr, "Eeek");
  if (capability == OI)
    return (OiCapability*)self;
  if (!res)
    {
      fprintf (stderr, "assert failes, object %p doesn't have capability \"%s\".\n",
               self, capability->name);
      *((Oi*)(NULL)) = *self; /* segfault so a backtrace is meaningful */
    }
  return res;
}

void *capability_ensure (OiType *capability,
                         Oi     *args)
{
  OiCapability *res = self@oi:capability_get (capability);
  if (self->capability_count == -66) fprintf (stderr, "Eeek");
  if (capability == OI)
    return (OiCapability*)self;
  if (!res)
    {
      self@oi:capability_add (capability, args);
      res = self@oi:capability_get (capability);
    }
  return res;
}

#define ALLOC_CHUNK   16
#define ALLOC_CHUNK_1 ALLOC_CHUNK-1

void capability_add (OiType *type,
                     Oi     *args)
{
  if (type == OI)
    return;
  if (self->capability_count == -66) fprintf (stderr, "Eeek");
  if (self@oi:capability_check (type))
    {
      fprintf (stderr, "Object %p already have capability \"%s\"\n",
               self, type->name);
      return;
    }
  if (
       ((self->capability_count + ALLOC_CHUNK)/ALLOC_CHUNK) * ALLOC_CHUNK >
       ((self->capability_count + ALLOC_CHUNK_1)/ALLOC_CHUNK) * ALLOC_CHUNK)
    {
      if (self->capabilities == NULL)
        self->capabilities = malloc (sizeof (OiCapability*) * ALLOC_CHUNK);
      else
        self->capabilities = realloc (self->capabilities, sizeof (OiCapability*) *
                             ((self->capability_count + ALLOC_CHUNK)/ALLOC_CHUNK)*ALLOC_CHUNK );
    }

  self->capabilities[self->capability_count] = oi_malloc (type->size);
  self->capabilities[self->capability_count]->type = type;
  if (type->init)
    type->init (self, self->capabilities[self->capability_count], args);
  if (type->init_int)
    type->init_int (self, self->capabilities[self->capability_count]);

  self->capability_count++;

  self@message:emit ("oi:add-capability", type);
}

static void capability_destroy (OiCapability *capability)
{
  if (self->capability_count == -66) fprintf (stderr, "Eeek");
  if (capability->type->destroy)
    capability->type->destroy (self, capability);
  oi_free (capability->type->size, capability);
}

void capability_remove (OiType *capability)
{
  int i;
  if (capability == OI)
    return;
  if (self->capability_count == -66) fprintf (stderr, "Eeek");
  if (!oi_capability_check (self, capability))
    {
      fprintf (stderr, "Object %p doesn't have capability \"%s\"\n", self, capability->name);
      return;
    };

  self@message:emit ("oi:remove-capability", capability);
  for (i = 0; i < self->capability_count; i++)
    if (self->capabilities[i]->type == capability)
      {
        int j;
        self@oi:capability_destroy (self->capabilities[i]);
        self->capability_count--;
        for (j = i; j < self->capability_count; j++)
          self->capabilities[j] = self->capabilities[j+1];
        return;
      }
}

void finalize ()
{
  int i;
  self@message:emit ("oi:die", NULL);
  for (i = self->capability_count-1; i>=0 ; i--)
    self@oi:capability_destroy (self->capabilities[i]);
  free (self->capabilities);
  self->capability_count = -66;
  oi_free (sizeof (Oi), self);
}

const OiCapability **oi_capability_list (int *count)
{
  if (count)
    *count = self->capability_count;
  return (void*)self->capabilities;
}

@end

Oi * oi_new (void)
{
  Oi *self = oi_malloc (sizeof(Oi));
  self->capabilities = NULL;
  self->capability_count = 0;
  return self;
}

Oi *oi_new_bare (OiType *type, void *userdata)
{
  Oi *self = @oi:new ();
  self@oi:capability_add (type, userdata);
  return self;
}
