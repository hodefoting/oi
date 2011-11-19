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

typedef struct
{
  OiCapability  capability;
  void   (*destroy)(void *item, void *user_data);
  void    *destroy_data;
  void   **items;
  int      size;
} List;

static void
list_init (Oi     *oi,
           OiCapability *capability,
           Oi     *args)
{
  List *list  = (List*)capability;
  list->items = NULL;
  list->destroy = NULL;
  list->size  = 0;
}

static void
list_destroy (Oi      *oi,
              OiCapability *capability)
{
  List *list = (List*)capability;
  if (list->destroy)
    oi_list_each (oi, list->destroy, list->destroy_data);
  if (list->items)
    free (list->items);
}
OI(LIST, List, NULL, list_init, list_destroy)
#define OI_LIST(oi) ((List*)oi_capability_get_assert (oi, LIST))

Oi *oi_list_new (void)
{
  Oi *oi       = oi_new ();
  oi_capability_add (oi, LIST, NULL);
  return oi;
}

int
oi_list_get_size (Oi *oi)
{
  return OI_LIST(oi)->size;
}

#define CS 8
#define CS_1 (CS-1)

void
oi_list_append (Oi *oi, void *data)
{
  List *list = OI_LIST (oi);

  if (((list->size + CS)/CS) * CS >
      ((list->size + CS_1)/CS) * CS)
    {
      if (list->items == NULL)
        list->items = malloc (sizeof (void*) * CS);
      else
        list->items = realloc (list->items, sizeof (OiCapability*) *
                              ((list->size + CS)/CS)*CS);
    }
  list->items[list->size] = data;
  list->size++;
}

void  *
oi_list_get (Oi *oi, int no)
{
  List *list = OI_LIST (oi);
  if (no >= 0 && no < list->size)
    return list->items[no];
  return NULL;
}

void
oi_list_each (Oi *oi,
              void (*cb)(void *item, void *user_data),
              void *user_data)
{
  List *list = OI_LIST (oi);
  int i;
  for (i = 0; i < list->size; i++)
    {
      cb (list->items[i], user_data);
    }
}

void
oi_list_remove_index_fast (Oi *oi,
                           int index)
{
  List *list = OI_LIST (oi);
  if (!(index >= 0 && index < list->size))
    return;

  if (list->destroy)
    list->destroy (list->items[index], list->destroy_data);
  list->items[index] = list->items[list->size-1];
  list->size--;
}

void
oi_list_remove_index (Oi *oi,
                      int index)
{
  List *list = OI_LIST (oi);
  int j;
  if (!(index >= 0 && index < list->size))
    return;

  if (list->destroy)
    list->destroy (list->items[index], list->destroy_data);
  list->size--;
  for (j = index; j < list->size; j++)
    list->items[j] = list->items[j+1];
}

void
oi_list_remove (Oi   *oi,
                void *data)
{
  List *list = OI_LIST (oi);
  int i;
  for (i = 0; i < list->size; i++)
    if (list->items[i] == data)
      {
        oi_list_remove_index (oi, i);
        return;
      }
}

void
oi_list_remove_fast (Oi   *oi,
                     void *data)
{
  List *list = OI_LIST (oi);
  int i;
  for (i = 0; i < list->size; i++)
    if (list->items[i] == data)
      {
        oi_list_remove_index_fast (oi, i);
        return;
      }
}

void
oi_list_remove_zombie_index_fast (Oi *oi,
                                  int index)
{
  List *list = OI_LIST (oi);
  if (!(index >= 0 && index < list->size))
    return;

  list->items[index] = list->items[list->size-1];
  list->size--;
}
void
oi_list_remove_zombie_index (Oi *oi,
                             int index)
{
  List *list = OI_LIST (oi);
  int j;
  if (!(index >= 0 && index < list->size))
    return;
  list->size--;
  for (j = index; j < list->size; j++)
    list->items[j] = list->items[j+1];
}

void
oi_list_remove_zombie (Oi   *oi,
                       void *data)
{
  List *list = OI_LIST (oi);
  int i;
  for (i = 0; i < list->size; i++)
    if (list->items[i] == data)
      {
        oi_list_remove_zombie_index (oi, i);
        return;
      }
}

void
oi_list_remove_zombie_fast (Oi   *oi,
                     void *data)
{
  List *list = OI_LIST (oi);
  int i;
  for (i = 0; i < list->size; i++)
    if (list->items[i] == data)
      {
        oi_list_remove_zombie_index_fast (oi, i);
        return;
      }
}

void oi_list_set_destroy (Oi *oi,
                          void (*destroy)(void *item, void *user_data),
                          void *user_data)
{
  List *list = OI_LIST (oi);
  list->destroy = destroy;
  list->destroy_data = user_data;
}

int oi_list_find_custom  (Oi *oi, int (*match_fun)(void *item, void *user_data),
                                      void *user_data)
{
  List *list = OI_LIST (oi);
  int i;
  for (i = 0; i < list->size; i++)
    if (match_fun (list->items[i], user_data))
      return i;
  return -1;
}

static int match_direct (void *item, void *user_data)
{
  if (item == user_data)
    return 1;
  return 0;
}

int oi_list_find         (Oi *oi, void *data)
{
  return oi_list_find_custom (oi, match_direct, data);
}
