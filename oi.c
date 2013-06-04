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
  int             trait_count;
  /* XXX: this could be a treap */
  OiTrait  **traits;
};

/* checks if the object has the given instance */
int
trait_check (OiType *trait)
{
  int i;
  if (self->trait_count == -66) fprintf (stderr, "Eeek");
  if (trait == OI)
    return 1;
  for (i = 0; i < self->trait_count; i++)
    if (self->traits[i]->type == trait)
      return 1;
  return 0;
}

/* gets the trait, if any */
void *trait_get (OiType *trait)
{
  int i;
  if (self->trait_count == -66) fprintf (stderr, "Eeek");
  if (trait == OI)
    return (OiTrait*)self;
  for (i = 0; i < self->trait_count; i++)
    if (self->traits[i]->type == trait)
      return self->traits[i];
  return NULL;
}

/* gets an trait, if trait doesn't already exist fail with warning 
 * (and segfault) */
void *trait_get_assert (OiType *trait)
{
  OiTrait *res = self@oi:trait_get (trait);
  if (self->trait_count == -66) fprintf (stderr, "Eeek");
  if (trait == OI)
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
void *trait_ensure (OiType *trait,
                         Oi     *args)
{
  OiTrait *res = self@oi:trait_get (trait);
  if (self->trait_count == -66) fprintf (stderr, "Eeek");
  if (trait == OI)
    return (OiTrait*)self;
  if (!res)
    {
      self@oi:trait_add (trait, args);
      res = self@oi:trait_get (trait);
    }
  return res;
}

#define ALLOC_CHUNK   16
#define ALLOC_CHUNK_1 ALLOC_CHUNK-1

/* adds an trait to an instance */
void trait_add (OiType *type,
                Oi     *args)
{
  if (type == OI)
    return;
  if (self->trait_count == -66) fprintf (stderr, "Eeek");
  if (self@oi:trait_check (type))
    {
      fprintf (stderr, "Object %p already have trait \"%s\"\n",
               self, type->name);
      return;
    }
  if (
       ((self->trait_count + ALLOC_CHUNK)/ALLOC_CHUNK) * ALLOC_CHUNK >
       ((self->trait_count + ALLOC_CHUNK_1)/ALLOC_CHUNK) * ALLOC_CHUNK)
    {
      if (self->traits == NULL)
        self->traits = malloc (sizeof (OiTrait*) * ALLOC_CHUNK);
      else
        self->traits = realloc (self->traits, sizeof (OiTrait*) *
                             ((self->trait_count + ALLOC_CHUNK)/ALLOC_CHUNK)*ALLOC_CHUNK );
    }

  self->traits[self->trait_count] = oi_malloc (type->size);
  self->traits[self->trait_count]->type = type;
  self->trait_count++;
  if (type->init)
    type->init (self, self->traits[self->trait_count-1], args);
  if (type->init_int)
    type->init_int (self, self->traits[self->trait_count-1]);


  self@message:emit ("oi:add-trait", type);
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
void trait_remove (OiType *trait)
{
  int i;
  if (trait == OI)
    return;
  if (self->trait_count == -66) fprintf (stderr, "Eeek");
  if (!oi_trait_check (self, trait))
    {
      fprintf (stderr, "Object %p doesn't have trait \"%s\"\n", self, trait->name);
      return;
    };

  self@message:emit ("oi:remove-trait", trait);
  for (i = 0; i < self->trait_count; i++)
    if (self->traits[i]->type == trait)
      {
        int j;
        self@oi:trait_destroy (self->traits[i]);
        self->trait_count--;
        for (j = i; j < self->trait_count; j++)
          self->traits[j] = self->traits[j+1];
        return;
      }
}

/* used to implement the object reaping side of oi_unref; do not use
 * directly
 */
void finalize ()
{
  int i;
  self@message:emit ("oi:die", NULL);
  for (i = self->trait_count-1; i>=0 ; i--)
    self@oi:trait_destroy (self->traits[i]);
  free (self->traits);
  self->trait_count = -66;
  oi_free (sizeof (Oi), self);
}

/* get a list of traits, the returned list of pointers is NULL terminated
 * and should not be freed by the caller.
 */
const OiTrait **oi_trait_list (int *count)
{
  if (count)
    *count = self->trait_count;
  return (void*)self->traits;
}

@end

Oi * oi_new (void)
{
  Oi *self = oi_malloc (sizeof(Oi));
  self->traits = NULL;
  self->trait_count = 0;
  return self;
}

Oi *oi_new_bare (OiType *type, void *userdata)
{
  Oi *self = @oi:new ();
  self@oi:trait_add (type, userdata);
  return self;
}

Oi *oi_make_args (Oi *program, char **argv)
{
  Oi *ret = list_new ();
  program@["name"oi]=string_new(argv[0]);
  argv++;
  while (*argv)
    {
      ret@list:append (string_new (*argv));
      argv++;
    }
  program@["args"oi]=ret;
  return ret;
}
