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
  int      trait_count;
  /* XXX: this could be a treap */
  Trait  **traits;
} PACKED;

#define DEATH_MARK   -66

static inline void check_dead ()
{
  if (!self)
    return;
  if (self->trait_count == DEATH_MARK) fprintf (stderr, "Eeek");
}

int is_dead ()
{
  return (self->trait_count == DEATH_MARK);
}

/* gets the trait, if any */
void *get (Type *trait)
{
  int i;
  if (!self)
    return NULL;
  self@trait:check_dead ();
  if (trait == TRAIT)
    return (Trait*)self;
  for (i = 0; i < self->trait_count; i++)
    if (self->traits[i]->trait_type == trait)
      return self->traits[i];
  return NULL;
}

/* gets an trait, if trait doesn't already exist fail with warning 
 * (and segfault) */
void *get_assert (Type *trait)
{
  Trait *res = self@trait:get (trait);
  self@trait:check_dead ();
  if (trait == TRAIT)
    return (Trait*)self;
  if (!res)
    {
      fprintf (stderr, "assert failes, object %p doesn't have trait \"%s\".\n",
               self, trait->name);
      *((var)(NULL)) = *self; /* segfault so a backtrace is meaningful */
    }
  return res;
}

#define ALLOC_CHUNK   4

/* adds an trait to an instance */
static void add (Type *type, var args)
{
  if (!self)
    return;
  self@trait:check_dead ();
  if (type == TRAIT)
    return;
  if (self@trait:get (type))
    {
      fprintf (stderr, "Object %p already has trait \"%s\"\n",
               self, type->name);
      return;
    }
  if (
       ((self->trait_count + ALLOC_CHUNK)/ALLOC_CHUNK) * ALLOC_CHUNK >
       ((self->trait_count + (ALLOC_CHUNK-1))/ALLOC_CHUNK) * ALLOC_CHUNK)
    {
      if (self->traits == NULL)
        self->traits = oi_malloc (sizeof (Trait*) * ALLOC_CHUNK);
      else
        self->traits = oi_realloc (self->traits, sizeof (Trait*) *
                             ((self->trait_count + ALLOC_CHUNK)/ALLOC_CHUNK)*ALLOC_CHUNK );
    }

  if (type->init_pre)
    type->init_pre (self);

  int trait_no = self->trait_count;
  self->trait_count++;
  self->traits[trait_no] = oi_malloc (type->size);
  self->traits[trait_no]->trait_type = type;
  if (type->init)
    type->init (self, self->traits[trait_no], args);
  if (type->init_int)
    type->init_int (self, self->traits[trait_no]);

  self@"oi:add-trait"(type);
}

/* gets the trait, creates and adds it if it doesn't already exist */
void *ensure (Type *trait, var args)
{
  Trait *res = self@trait:get (trait);
  self@trait:check_dead ();
  if (trait == TRAIT)
    return (Trait*)self;
  if (!res)
    {
      self@trait:add (trait, args);
      res = self@trait:get (trait);
    }
  return res;
}


static void trait_destroy (Trait *trait)
{
  self@trait:check_dead ();
  if (trait->trait_type->destroy)
    trait->trait_type->destroy (self, trait);
  oi_free (trait->trait_type->size, trait);
}

/* remove a trait from an instance */
void remove (Type *trait)
{
  int i;
  self@trait:check_dead ();
  if (trait == TRAIT)
    return;
  if (!trait_get (self, trait))
    {
      fprintf (stderr, "Object %p doesn't have trait \"%s\"\n", self, trait->name);
      return;
    };

  self@"oi:remove-trait"(trait);
  for (i = 0; i < self->trait_count; i++)
    if (self->traits[i]->trait_type == trait)
      {
        int j;
        self@trait:trait_destroy (self->traits[i]);
        self->trait_count--;
        for (j = i; j < self->trait_count; j++)
          self->traits[j] = self->traits[j+1];
        return;
      }
}

static void finalize ()
{
  int i;
  self@trait:check_dead ();
  self@"oi:die"(NULL);
  for (i = self->trait_count-1; i>=0 ; i--)
    self@trait:trait_destroy (self->traits[i]);
  oi_free (((self->trait_count + ALLOC_CHUNK)/ALLOC_CHUNK)*ALLOC_CHUNK,
        self->traits);
  self->trait_count = DEATH_MARK;
  oi_free (sizeof (var), self);
}

/* get a list of traits, the returned list of pointers is NULL terminated
 * and should not be freed by the caller.
 */
static void **list_int (int *count)
{
  if (count)
    *count = self->trait_count;
  return (void*)self->traits;
}

var list ()
{
  var ret = var_new(LIST, NULL);
  int i;
  int count;
  Type ***tlist = (void*)self@trait:list_int (&count);
  for (i = 0; i < count; i ++)
     {
       ret@list:append((void*)(*(*tlist[i])).name);
     }
  return ret;
}

/* used to implement the object reaping side of oi_unref; do not use
 * directly
 */
void var_finalize! (var self)
{
  trait_finalize (self);
}

var var_new! (Type *type, void *args)
{
  var self = oi_malloc (sizeof(Trait));
  self->traits = NULL;
  self->trait_count = 0;
  if (type)
    self@trait:add (type, args);
  return self;
}

var var_dup! (var self)
{
  if (!self)
    return NULL;
  var clone = var_new (NULL, NULL);

  {
    int count;
    int i;
    Type ***tlist = (void*)self@trait:list_int (&count);
    self@property:dup (clone);

    for (i = 0; i < count; i ++)
       {
         /* XXX: is this sufficient; is this or even always safe?*/
         clone@trait:ensure (((*tlist[i])), NULL);
         if ((*(*tlist[i])).dup)
           {
             (*(*tlist[i])).dup (self, clone);
           }
         else
           {
             /* XXX: unhandled trait in dup.. */
             //fprintf (stderr, "%s: ", (*(*tlist[i])).name);
           }
       }
  }
  return clone;
}

@end

