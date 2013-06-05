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
};

#define CS 8

static void init ()
{
  list->items = NULL;
  list->destroy = NULL;
  list->size  = 0;
}

/* void each (void (*cb)(void *item, void *user_data), void *user_data) */

void each (void *cbp, void *user_data)
{
  int i;
  void (*cb)(void *item, void *user_data) = cbp;
  List *list = self@trait:get_assert (LIST);
  for (i = 0; i < list->size; i++)
    cb (list->items[i], user_data);
}

static void destroy ()
{
  if (list->destroy)
    self@list:each (list->destroy, list->destroy_data);
  if (list->items)
    oi_free (((list->size + CS)/CS)*CS, list->items);
}

void * get (int no)
{
  List *list = self@trait:get_assert (LIST);
  if (no >= 0 && no < list->size)
    return list->items[no];
  return NULL;
}

Var remove_index_fast (int index)
{
  List *list = self@trait:get_assert (LIST);
  if (!(index >= 0 && index < list->size))
    return self;

  if (list->destroy)
    list->destroy (list->items[index], list->destroy_data);
  list->items[index] = list->items[list->size-1];
  list->size--;
  return self;
}

Var remove_index (int index)
{
  int j;
  List *list = self@trait:get_assert (LIST);
  if (!(index >= 0 && index < list->size))
    return self;

  if (list->destroy)
    list->destroy (list->items[index], list->destroy_data);
  list->size--;
  for (j = index; j < list->size; j++)
    list->items[j] = list->items[j+1];
  return self;
}

Var remove (void *data)
{
  List *list = self@trait:get_assert (LIST);
  int i;
  for (i = 0; i < list->size; i++)
    if (list->items[i] == data)
      {
        self@list:remove_index (i);
        return self;
      }
  return self;
}

int get_size ()
{
  List *list = self@trait:get_assert (LIST);
  return list->size;
}

Var list_remove_fast (void *data)
{
  List *list = self@trait:get_assert (LIST);
  int i;
  for (i = 0; i < list->size; i++)
    if (list->items[i] == data)
      {
        self@list:remove_index_fast (i);
        return self;
      }
  return self;
}

Var remove_zombie_index_fast (int index)
{
  List *list = self@trait:get_assert (LIST);
  if (!(index >= 0 && index < list->size))
    return self;

  list->items[index] = list->items[list->size-1];
  list->size--;
  return self;
}

Var remove_zombie_index (int index)
{
  List *list = self@trait:get_assert (LIST);
  int j;
  if (!(index >= 0 && index < list->size))
    return self;
  list->size--;
  for (j = index; j < list->size; j++)
    list->items[j] = list->items[j+1];
  return self;
}

Var remove_zombie (void *data)
{
  List *list = self@trait:get_assert (LIST);
  int i;
  for (i = 0; i < list->size; i++)
    if (list->items[i] == data)
      {
        self@list:remove_zombie_index (i);
        return self;
      }
  return self;
}

Var remove_zombie_fast (void *data)
{
  List *list = self@trait:get_assert (LIST);
  int i;
  for (i = 0; i < list->size; i++)
    if (list->items[i] == data)
      {
        self@list:remove_zombie_index_fast (i);
        return self;
      }
  return self;
}

/*
Var *set_destroy (void (*destroy)(void *item, void *user_data),
                  void *user_data)
                  */

Var set_destroy (void *destroy, void *user_data)
{
  List *list = self@trait:get_assert (LIST);
  list->destroy = destroy;
  list->destroy_data = user_data;
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
  List *list = self@trait:get_assert (LIST);
  int i;
  for (i = 0; i < list->size; i++)
    if (match_fun (list->items[i], user_data))
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


Var append (void *data)
{
  List *list = self@trait:get_assert (LIST);

  if (((list->size + CS)/CS) * CS >
      ((list->size + (CS-1))/CS) * CS)
    {
      if (list->items == NULL)
        list->items = oi_malloc (sizeof (void*) * CS);
      else
        list->items = oi_realloc (list->items, sizeof (Type*) *
                                  ((list->size + CS)/CS)*CS);
    }
  list->items[list->size] = data;
  list->size++;

  return self;
}

Var new ()
{
  return var_new (LIST, NULL);
}

@end
