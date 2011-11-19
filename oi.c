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

struct _Oi
{
  int           capability_count;
  /* this could be a treap */
  OiCapability      **capabilities;
} __attribute((packed));

int
oi_capability_check (Oi     *oi,
             OiType *capability)
{
  int i;
  for (i = 0; i < oi->capability_count; i++)
    if (oi->capabilities[i]->type == capability)
      return 1;
  return 0;
}

OiCapability *
oi_capability_get (Oi          *oi,
              OiType *capability)

{
  int i;
  for (i = 0; i < oi->capability_count; i++)
    if (oi->capabilities[i]->type == capability)
      return oi->capabilities[i];
  return NULL;
}

OiCapability *
oi_capability_get_assert (Oi          *oi,
                     OiType *capability)

{
  OiCapability *res = oi_capability_get (oi, capability);
  if (!res)
    {
      fprintf (stderr, "assert failes, object %p doesn't have capability \"%s\", if you did not expect this to happen,. it could have been caused by forgetting to pad the start of a capability instance struct.\n",
               oi, capability->name);
      *((Oi*)(NULL)) = *oi; /* segfault so a backtrace is meaningful */
    }
  return res;
}

OiCapability *oi_capability_ensure (Oi     *oi,
                            OiType *capability,
                            Oi     *args)
{
  OiCapability *res = oi_capability_get (oi, capability);
  if (!res)
    {
      oi_capability_add (oi, capability, args);
      res = oi_capability_get (oi, capability);
    }
  return res;
}

#define ALLOC_CHUNK   16
#define ALLOC_CHUNK_1 ALLOC_CHUNK-1

void oi_capability_add (Oi     *oi,
                  OiType *type,
                  Oi     *args)
{
  if (oi_capability_check (oi, type))
    {
      fprintf (stderr, "Object %p already have capability \"%s\"\n",
               oi, type->name);
      return;
    }
  if (
       ((oi->capability_count + ALLOC_CHUNK)/ALLOC_CHUNK) * ALLOC_CHUNK >
       ((oi->capability_count + ALLOC_CHUNK_1)/ALLOC_CHUNK) * ALLOC_CHUNK)
    {
      if (oi->capabilities == NULL)
        oi->capabilities = malloc (sizeof (OiCapability*) * ALLOC_CHUNK);
      else
        oi->capabilities = realloc (oi->capabilities, sizeof (OiCapability*) *
                             ((oi->capability_count + ALLOC_CHUNK)/ALLOC_CHUNK)*ALLOC_CHUNK );
    }
  oi->capabilities[oi->capability_count] = oi_malloc (type->size);
  oi->capabilities[oi->capability_count]->type = type;
  oi->capability_count++;
  if (type->init)
    type->init (oi, oi->capabilities[oi->capability_count-1], args);
  oi_message_emit (oi, "oi:add-capability", type);
}

static void
oi_capability_destroy (Oi *oi, OiCapability *capability)
{
  if (capability->type->destroy)
    capability->type->destroy (oi, capability);
  oi_free (capability->type->size, capability);
}

void
oi_capability_remove (Oi           *oi,
                 OiType *capability)
{
  int i;
  if (!oi_capability_check (oi, capability))
    {
      fprintf (stderr, "Object %p doesn't have capability \"%s\"\n", oi, capability->name);
      return;
    }
  oi_message_emit (oi, "oi:remove-capability", capability);
  for (i = 0; i < oi->capability_count; i++)
    if (oi->capabilities[i]->type == capability)
      {
        int j;
        oi_capability_destroy (oi, oi->capabilities[i]);
        oi->capability_count--;
        for (j = i; j < oi->capability_count; j++)
          oi->capabilities[j] = oi->capabilities[j+1];
        return;
      }
}

Oi *
oi_new (void)
{
  Oi *oi = oi_malloc (sizeof(Oi));
  oi->capabilities = NULL;
  oi->capability_count = 0;
  return oi;
}

void
oi_destroy (Oi *oi)
{
  int i;
  oi_message_emit (oi, "oi:die", NULL);
  for (i = oi->capability_count-1; i>=0 ; i--)
    oi_capability_destroy (oi, oi->capabilities[i]);
  free (oi->capabilities);
  oi->capability_count = -66;
  oi_free (sizeof (Oi), oi);
}

const OiCapability **oi_capability_list (Oi *oi, int *count)
{
  if (count)
    *count = oi->capability_count;
  return (void*)oi->capabilities;
}
