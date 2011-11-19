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

#ifndef OI_PROPERTIES_H
#define OI_PROPERTIES_H

extern OiType     *PROPERTIES;

/* implement all conversions, and make it impossible to change the type
   of a property after it has been first instantiated? (not permitting changes
   would make it possible to use varargs effectively
 */

void   oi_set_float       (Oi *oi, const char *name, float       value);
void   oi_set_int         (Oi *oi, const char *name, int         value);
void   oi_set_string      (Oi *oi, const char *name, const char *value);
void   oi_set_pointer     (Oi *oi, const char *name, void       *ptr);
void   oi_set_oi          (Oi *oi, const char *name, Oi         *value);
float  oi_get_float       (Oi *oi, const char *name);
int    oi_get_int         (Oi *oi, const char *name);
const char *oi_get_string (Oi *oi, const char *name);
void  *oi_get_pointer     (Oi *oi, const char *name);
Oi    *oi_get_oi          (Oi *oi, const char *name);


void oi_properties_each (Oi *oi,
                         void (*cb)(const char *name, void *value,
                                    void *user_data),
                         void *user_data);
int oi_property_is_string (Oi *oi, const char *name);

#endif
