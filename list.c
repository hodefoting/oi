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

#include "oi.h"
#include <stdlib.h>

@genearateheader

@trait List
{
  void (*destroy)(void *item, void *user_data);
  void  *destroy_data;
  void **items;
  int    size;
} PACKED;

#define CS 8

static void init ()
{
  this->items = NULL;
  this->destroy = NULL;
  this->size  = 0;
}

/* void each (void (*cb)(void *item, void *user_data), void *user_data) */

void each (void *cbp, void *user_data)
{
  int i;
  void (*cb)(void *item, void *user_data) = cbp;
  for (i = 0; i < this->size; i++)
    cb (this->items[i], user_data);
}

static void destroy ()
{
  if (this->destroy)
    self@list:each (this->destroy, this->destroy_data);
  if (this->items)
    oi_free (((this->size + CS)/CS)*CS, this->items);
}

void * get (int no)
{
  if (no >= 0 && no < this->size)
    return this->items[no];
  return NULL;
}

var remove_index_fast (int index)
{
  if (!(index >= 0 && index < this->size))
    return self;

  if (this->destroy)
    this->destroy (this->items[index], this->destroy_data);
  this->items[index] = this->items[this->size-1];
  this->size--;
  return self;
}

var remove_index (int index)
{
  int j;
  if (!(index >= 0 && index < this->size))
    return self;

  if (this->destroy)
    this->destroy (this->items[index], this->destroy_data);
  this->size--;
  for (j = index; j < this->size; j++)
    this->items[j] = this->items[j+1];
  return self;
}

var remove (void *data)
{
  int i;
  for (i = 0; i < this->size; i++)
    if (this->items[i] == data)
      {
        self@list:remove_index (i);
        return self;
      }
  return self;
}

int get_size ()
{
  return this->size;
}

var remove_fast (void *data)
{
  int i;
  for (i = 0; i < this->size; i++)
    if (this->items[i] == data)
      {
        self@list:remove_index_fast (i);
        return self;
      }
  return self;
}

var remove_zombie_index_fast (int index)
{
  if (!(index >= 0 && index < this->size))
    return self;

  this->items[index] = this->items[this->size-1];
  this->size--;
  return self;
}

var remove_zombie_index (int index)
{
  int j;
  if (!(index >= 0 && index < this->size))
    return self;
  this->size--;
  for (j = index; j < this->size; j++)
    this->items[j] = this->items[j+1];
  return self;
}

var remove_zombie (void *data)
{
  int i;
  for (i = 0; i < this->size; i++)
    if (this->items[i] == data)
      {
        self@list:remove_zombie_index (i);
        return self;
      }
  return self;
}

var remove_zombie_fast (void *data)
{
  int i;
  for (i = 0; i < this->size; i++)
    if (this->items[i] == data)
      {
        self@list:remove_zombie_index_fast (i);
        return self;
      }
  return self;
}

/*
var *set_destroy (void (*destroy)(void *item, void *user_data),
                  void *user_data)
                  */

var set_destroy (void *destroy, void *user_data)
{
  this->destroy = destroy;
  this->destroy_data = user_data;
  return self;
}

/* XXX: fixme in the oiccc parser; so that function callbacks
 * can exist.
 *
int find_custom (int (*match_fun)(void *item, void *user_data),
                 void *user_data)
                 */

int find_custom (void *matchfunp,
                 void *user_data)
{
  int (*match_fun)(void *item, void *user_data) = matchfunp;
  int i;
  for (i = 0; i < this->size; i++)
    if (match_fun (this->items[i], user_data))
      return i;
  return -1;
}

static int match_direct! (void *item, void *user_data)
{
  if (item == user_data)
    return 1;
  return 0;
}

int find (void *data)
{
  return (self@list:find_custom (match_direct, data));
}

var append (void *data)
{
  if (((this->size + CS)/CS) * CS >
      ((this->size + (CS-1))/CS) * CS)
    {
      if (this->items == NULL)
        this->items = oi_malloc (sizeof (void*) * CS);
      else
        this->items = oi_realloc (this->items, sizeof (Type*) *
                                  ((this->size + CS)/CS)*CS);
    }
  this->items[this->size] = data;
  this->size++;

  return self;
}

var new ()
{
  return var_new (LIST, NULL);
}

@end
