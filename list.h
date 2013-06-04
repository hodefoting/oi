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

#ifndef OI_LIST_H
#define OI_LIST_H

extern  OiType          *LIST;
Oi     *list_new          (void); /* the list is a capability, but can also be used
                                    on its own */

int     list_get_size            (Oi *oi);
void    list_append              (Oi *oi, void *data);
void    list_remove              (Oi *oi, void *data);
void    list_remove_fast         (Oi *oi, void *data);
void    list_remove_index        (Oi *oi, int no);
void    list_remove_index_fast   (Oi *oi,
                                  int index);
void    list_remove_zombie       (Oi *oi, void *data);
void    list_remove_zombie_fast  (Oi *oi, void *data);
void    list_remove_zombie_index (Oi *oi, int no);
void    list_remove_zombie_index_fast (Oi *oi,
                                       int index);
int     list_find_custom  (Oi *oi, int (*match_fun)(void *item, void *user_data),
                                   void *user_data);
int     list_find         (Oi *oi, void *data);
void   *list_get          (Oi *oi, int no);
void    list_set_destroy  (Oi *oi, void (*destroy)(void *item, void *user_data),
                           void *user_data);
void    list_each         (Oi *oi, void (*cb)(void *item, void *user_data),
                           void *user_data);
float   list_get_foo      (Oi *oi);
void    list_set_foo      (Oi *oi, float foo);

#endif
