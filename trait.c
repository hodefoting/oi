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
#include "oi.h"

@generateheader

@trait Trait
{
  int        trait_count;
  /* XXX: this could be a treap */
  OiTrait  **traits;
};

/* checks if the object has the given instance */
int check (Type *trait)
{
  int i;
  if (self->trait_count == -66) fprintf (stderr, "Eeek");
  if (trait == TRAIT)
    return 1;
  for (i = 0; i < self->trait_count; i++)
    if (self->traits[i]->type == trait)
      return 1;
  return 0;
}

/* gets the trait, if any */
void *get (Type *trait)
{
  int i;
  if (self->trait_count == -66) fprintf (stderr, "Eeek");
  if (trait == TRAIT)
    return (OiTrait*)self;
  for (i = 0; i < self->trait_count; i++)
    if (self->traits[i]->type == trait)
      return self->traits[i];
  return NULL;
}

/* gets an trait, if trait doesn't already exist fail with warning 
 * (and segfault) */
void *get_assert (Type *trait)
{
  OiTrait *res = self@trait:get (trait);
  if (self->trait_count == -66) fprintf (stderr, "Eeek");
  if (trait == TRAIT)
    return (OiTrait*)self;
  if (!res)
    {
      fprintf (stderr, "assert failes, object %p doesn't have trait \"%s\".\n",
               self, trait->name);
      *((Oi*)(NULL)) = *self; /* segfault so a backtrace is meaningful */
    }
  return res;
}

/* gets the trait, creates and adds it if it doesn't already exist */
void *ensure (Type *trait, Oi *args)
{
  OiTrait *res = self@trait:get (trait);
  if (self->trait_count == -66) fprintf (stderr, "Eeek");
  if (trait == TRAIT)
    return (OiTrait*)self;
  if (!res)
    {
      self@trait:add (trait, args);
      res = self@trait:get (trait);
    }
  return res;
}

#define ALLOC_CHUNK   16

/* adds an trait to an instance */
void add (Type *type, Oi *args)
{
  if (type == TRAIT)
    return;
  if (self->trait_count == -66) fprintf (stderr, "Eeek");
  if (self@trait:check (type))
    {
      fprintf (stderr, "Object %p already have trait \"%s\"\n",
               self, type->name);
      return;
    }
  if (
       ((self->trait_count + ALLOC_CHUNK)/ALLOC_CHUNK) * ALLOC_CHUNK >
       ((self->trait_count + (ALLOC_CHUNK-1))/ALLOC_CHUNK) * ALLOC_CHUNK)
    {
      if (self->traits == NULL)
        self->traits = oi_malloc (sizeof (OiTrait*) * ALLOC_CHUNK);
      else
        self->traits = oi_realloc (self->traits, sizeof (OiTrait*) *
                             ((self->trait_count + ALLOC_CHUNK)/ALLOC_CHUNK)*ALLOC_CHUNK );
    }

  self->traits[self->trait_count] = oi_malloc (type->size);
  self->traits[self->trait_count]->type = type;
  self->trait_count++;
  if (type->init)
    type->init (self, self->traits[self->trait_count-1], args);
  if (type->init_int)
    type->init_int (self, self->traits[self->trait_count-1]);

  self@"oi:add-trait"(type);
}

static void trait_destroy (OiTrait *trait)
{
  if (self->trait_count == -66) fprintf (stderr, "Eeek");
  if (trait->type->destroy)
    trait->type->destroy (self, trait);
  oi_free (trait->type->size, trait);
}

/* remove a trait from an instance */
void remove (Type *trait)
{
  int i;
  if (trait == TRAIT)
    return;
  if (self->trait_count == -66) fprintf (stderr, "Eeek");
  if (!trait_check (self, trait))
    {
      fprintf (stderr, "Object %p doesn't have trait \"%s\"\n", self, trait->name);
      return;
    };

  self@"oi:remove-trait"(trait);
  for (i = 0; i < self->trait_count; i++)
    if (self->traits[i]->type == trait)
      {
        int j;
        self@trait:trait_destroy (self->traits[i]);
        self->trait_count--;
        for (j = i; j < self->trait_count; j++)
          self->traits[j] = self->traits[j+1];
        return;
      }
}

void finalize ()
{
  int i;
  self@"oi:die"(NULL);
  for (i = self->trait_count-1; i>=0 ; i--)
    self@trait:trait_destroy (self->traits[i]);
  oi_free (((self->trait_count + ALLOC_CHUNK)/ALLOC_CHUNK)*ALLOC_CHUNK,
        self->traits);
  self->trait_count = -66;
  oi_free (sizeof (Oi), self);
}

/* get a list of traits, the returned list of pointers is NULL terminated
 * and should not be freed by the caller.
 */
const OiTrait **list (int *count)
{
  if (count)
    *count = self->trait_count;
  return (void*)self->traits;
}
@end

/* used to implement the object reaping side of oi_unref; do not use
 * directly
 */
void oi_finalize (Oi *self)
{
  trait_finalize (self);
}

Oi * oi_new (void)
{
  Oi *self = oi_malloc (sizeof(Oi));
  self->traits = NULL;
  self->trait_count = 0;
  return self;
}

Oi *oi_new_bare (Type *type, void *userdata)
{
  Oi *self = @oi:new ();
  self@trait:add (type, userdata);
  return self;
}