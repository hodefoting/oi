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

#include <stdlib.h>
#include <unistd.h>
#include "oi.h"
#include "pthread.h"


extern OiType     *LISTENER;
void oi_listener_add (Oi *oi, OiCapability *capability, const char *name, int id);

typedef struct
{
  const char *message_name;
  void  (*callback) (Oi *self, void *arg, void *user_data);
  void  *user_data;
} MessageEntry;

typedef struct
{
  OiCapability capability;
  Oi  *callbacks;
} Message;

static void message_init    (Oi *oi, OiCapability *capability, Oi *args);
static void message_destroy (Oi *oi, OiCapability *capability);
OI(MESSAGE, Message, NULL, message_init, message_destroy)
#define OI_MESSAGE(oi) ((Message*)oi_capability_get (oi, MESSAGE))

static void free_sentry (void *sentry)
{
  oi_free (sizeof (MessageEntry), sentry);
}

static void message_init (Oi *oi, OiCapability *capability, Oi *args)
{
  Message *message = (Message*)capability;
  message->callbacks = oi_list_new ();
  oi_list_set_destroy (message->callbacks, (void*) free_sentry, NULL);
}
static void message_destroy (Oi *oi, OiCapability *capability)
{
  Message *message = (Message*)capability;
  oi_destroy (message->callbacks);
}

static void
oi_emit_matching (void *entr,
                  void *data)
{
  MessageEntry *entry = entr;
  void **emit_data = data;
  if (entr && entry->message_name && entry->callback && !strcmp (emit_data[1], entry->message_name))
    entry->callback (emit_data[0], emit_data[2], entry->user_data);
}

void
oi_message_emit (Oi        *oi,
                const char *message_name,
                void       *arg)
{
  void *emit_data[3] = {oi, (void*)message_name, arg};
  if (oi_capability_check (oi, MESSAGE))
    oi_list_each (OI_MESSAGE(oi)->callbacks, oi_emit_matching, emit_data);
}

static Oi *queue = NULL;
static Oi *gq (void);
static void oi_queue_dispatcher (void *data)
{
  while (1)
    {
      while (oi_list_get_size (queue))
        {
          Oi *i;
          Oi *oi;
          const char *message_name;
          void *arg;
          void (*closure) (void *arg);

          //oi_lock (queue);
          i = oi_list_get (queue, 0);
          oi = oi_get_oi (i, "oi");
          message_name = oi_get_string (i, "message");
          arg = oi_get_pointer (i, "arg");
          closure = oi_get_pointer (i, "closure");
          oi_message_emit (oi, message_name, arg);
          oi_unref (oi);
          oi_list_remove_index (queue, 0);

          if (closure)
            closure (arg);
          //oi_unlock (queue);
        }
      usleep (1000);
    }
}

/* a singleton that returns the queue object, which contains its own
 * dispatcher thread
 */
static Oi *gq (void)
{
  if (!queue)
    {
      pthread_t thread;
      queue = oi_new ();
      oi_lock (queue);
      oi_capability_add (queue, LIST, NULL);
      oi_list_set_destroy (queue, (void*)oi_unref, NULL);
      oi_unlock (queue);
      pthread_create (&thread, NULL, (void*)oi_queue_dispatcher, NULL);
    }
  return queue;
}

void
oi_message_emit_remote (Oi         *oi,
                       const char *message_name,
                       void       *arg,
                       void (*closure) (void *arg))
{
  Oi *item = oi_new ();
  oi_lock (gq ());
  oi_set_oi      (item, "oi", oi);
  oi_set_string  (item, "message", message_name);
  oi_set_pointer (item, "arg", arg);
  oi_set_pointer (item, "closure", closure);
  oi_list_append (gq (), item);
  oi_unlock (gq ());
}

int
oi_message_listen (Oi         *oi,
                   Oi         *oi_self,
                   OiCapability     *capability_self,
                   const char *message_name,
                   void      (*callback) (Oi *self, void *arg, void *user_data),
                   void       *user_data)
{
  Message *message = (Message*)oi_capability_ensure (oi, MESSAGE, NULL);
  MessageEntry *entry;
  entry = oi_malloc (sizeof (MessageEntry));
  entry->message_name = message_name;
  entry->callback = callback;
  entry->user_data = user_data;
  oi_list_append (message->callbacks, entry);
  oi_message_emit (oi, "oi:message-connect", (void*)message_name);
  if (oi_self)
    oi_listener_add (oi_self, capability_self, message_name, oi_list_get_size (message->callbacks)-1);
  return oi_list_get_size (message->callbacks) - 1;
}

void
oi_message_handler_disconnect (Oi *oi,
                              int handler_id)
{
  Message *message = OI_MESSAGE (oi);
  if (!message)
    return;
  oi_message_emit (oi, "oi:message-disconnect", (void*)((MessageEntry*)oi_list_get (message->callbacks, handler_id))->message_name);
  oi_list_remove_index (message->callbacks, handler_id);
}

static int
match_func (void *entryp, void *callback)
{
  MessageEntry *entry = entryp;
  if (entry->callback == callback)
    return 1;
  return 0;
}

void   oi_message_handler_disconnect_by_func (Oi *oi,
                          void (*callback) (Oi *self, void *arg, void *user_data))
{
  Message *message = OI_MESSAGE (oi);
  int no;
  if (!message)
    return;
  no = oi_list_find_custom (message->callbacks, match_func, callback);
  if (no>=0)
    oi_message_handler_disconnect (oi, no);
}

/*** the following is the listener used for auto reaping of message handlers ***/

typedef struct
{
  OiType *type;
  char        *name;
  int          id;
} ListenerEntry;

typedef struct
{
  OiCapability capability;
  Oi     *listeners;
} Listener;
#define OI_LISTENER(oi) ((void*)oi_capability_get_assert (oi, LISTENER))

static void listener_remove_capability_cb (Oi *oi, void *arg, void *user_data)
{
  Listener *listener = OI_LISTENER (oi);
  OiType *type = arg;
  int i;
again:
  for (i = 0; i < oi_list_get_size (listener->listeners); i++)
    {
      ListenerEntry *entry = oi_list_get (listener->listeners, i);
      if (entry->type == type)
        {
          oi_message_handler_disconnect (oi, entry->id);
          oi_list_remove (listener->listeners, entry);
          goto again;
        }
    }
  if (oi_list_get_size (listener->listeners) == 0)
    oi_capability_remove (oi, LISTENER);
    /* we are no longer needed, so remove ourself */
}

static void lnrfree (ListenerEntry *entry)
{
  if (entry->name)
    oi_strfree (entry->name);
  oi_free (sizeof (ListenerEntry), entry);
}

static void listener_init (Oi *oi, OiCapability *capability, Oi *args)
{
  Listener *listener = (Listener*)capability;
  listener->listeners = oi_list_new ();
  oi_list_set_destroy (listener->listeners, (void*)lnrfree, NULL);
  oi_message_listen (oi, NULL, NULL, "oi:remove-capability", (void*)listener_remove_capability_cb, NULL);
}

static void listener_destroy (Oi *oi, OiCapability *capability)
{
  Listener *listener = (Listener*)capability;
  oi_destroy (listener->listeners);
  oi_message_handler_disconnect_by_func (oi, listener_remove_capability_cb);
}
OI(LISTENER, Listener, NULL, listener_init, listener_destroy)

static int listener_match_capability_and_name (void *listener_entry, void *arg)
{
  void **args=arg;
  ListenerEntry *entry = listener_entry;
  return (entry->type == args[1] && !strcmp (entry->name, args[0]));
}

static ListenerEntry *oi_get_entry_write (Oi *oi, OiType *type, const char *name)
{
  Listener *listener = ((void*)oi_capability_ensure (oi, LISTENER, NULL));
  int no;
  ListenerEntry *entry;
  void *args[]={(void*)name, type};
  no = oi_list_find_custom (listener->listeners, (void*)listener_match_capability_and_name, args);
  if (no >= 0)
    entry = oi_list_get (listener->listeners, no);
  else
    {
      entry = oi_malloc (sizeof (ListenerEntry));
      entry->name = oi_strdup (name);
      oi_list_append (listener->listeners, entry);
    }
  return entry;
}

void oi_listener_add (Oi *oi, OiCapability *capability, const char *name, int id)
{
  OiType *type = NULL;
  if (capability)
    type = capability->type;
  ListenerEntry *entry = oi_get_entry_write (oi, type, name);
  entry->type = type;
  entry->id = id;
}
