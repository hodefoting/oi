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

#ifndef OI_STRING_H
#define OI_STRING_H

/* ref-count is added to object on demand if the stringing functions are used */

extern OiType  *STRING;

Oi  *oi_string_new        (const char *initial);
void oi_string_clear      (Oi *oi);
void oi_string_appendc    (Oi *oi, int val);
void oi_string_append_str (Oi *oi, const char *str);
const char *oi_string_get (Oi *oi);
void oi_string_append_string (Oi *oi, Oi *oi2);

/* dissolving a string, means destroying it, but returning
 * the string, that should be manually freed.
 */
char *oi_string_dissolve  (Oi *oi);

#endif
