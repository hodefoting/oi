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

#include "oi.h"
#include <stdlib.h>

@trait List
{
  void (*destroy)(void *item, void *user_data);
  void  *destroy_data;
  void **items;
  int    size;
};

static void init ()
{
  list->items = NULL;
  list->destroy = NULL;
  list->size  = 0;
}

static void destroy ()
{
  if (list->destroy)
    self@list:each (list->destroy, list->destroy_data);
  if (list->items)
    free (list->items);
}

void  * get (int no)
{
  List *list = self@oi:trait_get_assert (LIST);
  if (no >= 0 && no < list->size)
    return list->items[no];
  return NULL;
}

void each (void (*cb)(void *item, void *user_data),
           void *user_data)
{
  int i;
  List *list = self@oi:trait_get_assert (LIST);
  for (i = 0; i < list->size; i++)
    cb (list->items[i], user_data);
}

void remove_index_fast (int index)
{
  List *list = self@oi:trait_get_assert (LIST);
  if (!(index >= 0 && index < list->size))
    return;

  if (list->destroy)
    list->destroy (list->items[index], list->destroy_data);
  list->items[index] = list->items[list->size-1];
  list->size--;
}

void remove_index (int index)
{
  int j;
  List *list = self@oi:trait_get_assert (LIST);
  if (!(index >= 0 && index < list->size))
    return;

  if (list->destroy)
    list->destroy (list->items[index], list->destroy_data);
  list->size--;
  for (j = index; j < list->size; j++)
    list->items[j] = list->items[j+1];
}

void remove (void *data)
{
  List *list = self@oi:trait_get_assert (LIST);
  int i;
  for (i = 0; i < list->size; i++)
    if (list->items[i] == data)
      {
        self@list:remove_index (i);
        return;
      }
}

int get_size ()
{
  List *list = self@oi:trait_get_assert (LIST);
  return list->size;
}

void list_remove_fast (void *data)
{
  List *list = self@oi:trait_get_assert (LIST);
  int i;
  for (i = 0; i < list->size; i++)
    if (list->items[i] == data)
      {
        self@list:remove_index_fast (i);
        return;
      }
}

void remove_zombie_index_fast (int index)
{
  List *list = self@oi:trait_get_assert (LIST);
  if (!(index >= 0 && index < list->size))
    return;

  list->items[index] = list->items[list->size-1];
  list->size--;
}

void remove_zombie_index (int index)
{
  List *list = self@oi:trait_get_assert (LIST);
  int j;
  if (!(index >= 0 && index < list->size))
    return;
  list->size--;
  for (j = index; j < list->size; j++)
    list->items[j] = list->items[j+1];
}

void remove_zombie (void *data)
{
  List *list = self@oi:trait_get_assert (LIST);
  int i;
  for (i = 0; i < list->size; i++)
    if (list->items[i] == data)
      {
        self@list:remove_zombie_index (i);
        return;
      }
}

void remove_zombie_fast (void *data)
{
  List *list = self@oi:trait_get_assert (LIST);
  int i;
  for (i = 0; i < list->size; i++)
    if (list->items[i] == data)
      {
        self@list:remove_zombie_index_fast (i);
        return;
      }
}

void set_destroy (void (*destroy)(void *item, void *user_data),
                  void *user_data)
{
  List *list = self@oi:trait_get_assert (LIST);
  list->destroy = destroy;
  list->destroy_data = user_data;
}

int find_custom (int (*match_fun)(void *item, void *user_data),
                 void *user_data)
{
  List *list = self@oi:trait_get_assert (LIST);
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

#define CS 8

void append (void *data)
{
  List *list = self@oi:trait_get_assert (LIST);

  if (((list->size + CS)/CS) * CS >
      ((list->size + (CS-1))/CS) * CS)
    {
      if (list->items == NULL)
        list->items = malloc (sizeof (void*) * CS);
      else
        list->items = realloc (list->items, sizeof (OiTrait*) *
                              ((list->size + CS)/CS)*CS);
    }
  list->items[list->size] = data;
  list->size++;
}

@end

Oi *list_new (void)
{
  return oi_new_bare (LIST, NULL);
}
