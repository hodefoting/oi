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

#ifndef message_H
#define message_H

/* The message trait is automatically instantiated for instances where
 * messages are attached, emitting messages on instances without any
 * attached handlers is a no-op
 */

extern OiType     *MESSAGE;

void   message_emit (Oi *oi, const char *message, void *data);

typedef void (*ClosureFunc)    (void* user_data);

#define CLOSURE(aa) (ClosureFunc)(aa)

/* XXX: should perhaps take a thread name, with all callbacks emited with
        the same name are serialized.
 */
void
message_emit_remote (Oi         *oi,
                       const char *message_name,
                       void       *arg,
                       void (*closure) (void *arg));

/*
 * if trait is passed, then the message is automatically disconnected when the
 * connecting trait is
 */
int
message_listen (Oi         *oi,
                Oi         *oi_self,
                OiTrait    *trait_self,
                const char *message_name,
                void      (*callback) (Oi *self, void *arg, void *user_data),
                void       *user_data);

void   message_handler_disconnect (Oi *oi,
                                     int handler_id);
void   message_handler_disconnect_by_func (Oi *oi,
                          void (*callback) (Oi *self, void *arg, void *user_data));
void   message_handler_disconnect_by_func_and_data (Oi *oi,
                          void (*callback) (Oi *self, void *arg, void *user_data));
#endif
