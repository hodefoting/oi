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
@generateheader

@trait MsgDisconnect
{
  Var *list;
};

typedef struct
{
  Type *type;
  char *name;
  int   id;
} ListenerEntry;

"oi:remove-trait" (void *arg, void *user_data)
{
  MsgDisconnect *msgdc = self@trait:get_assert(MSG_DISCONNECT);
  Type *type = arg;
  int i;
again:
  for (i = 0; i < (msgdc->list@list:get_size()); i++)
    {
      ListenerEntry *entry = msgdc->list@list:get(i);
      if (entry->type == type)
        {
          self@message:handler_disconnect (entry->id);
          msgdc->list@list:remove(entry);
          goto again;
        }
    }
  if (msgdc->list@list:get_size () == 0)
    self@trait:remove (MSG_DISCONNECT);
    /* we are no longer needed, so remove ourself */
  return 0;
}

static void lnrfree! (ListenerEntry *entry)
{
  if (entry->name)
    oi_strfree (entry->name);
  oi_free (sizeof (ListenerEntry), entry);
}

static void init ()
{
  msg_disconnect->list= @list:new ();
  msg_disconnect->list@list:set_destroy ((void*)lnrfree, NULL);
}

static void destroy ()
{
  msg_disconnect->list@var:finalize();

  self@message:handler_disconnect_by_func ((void*)msg_disconnect_oi_remove_trait_cb);
}

static int listener_match_trait_and_name! (void *listener_entry, void *arg)
{
  void **args=arg;
  ListenerEntry *entry = listener_entry;
  return (entry->type == args[1] && !strcmp (entry->name, args[0]));
}

static ListenerEntry *get_entry_write (Type *type, const char *name)
{
  MsgDisconnect *msgdc = ((void*)self@trait:ensure (MSG_DISCONNECT, NULL));
  int no;
  ListenerEntry *entry;
  void *args[]={(void*)name, type};
  no = msgdc->list@list:find_custom ((void*)listener_match_trait_and_name, args);
  if (no >= 0)
    entry = msgdc->list@list:get (no);
  else
    {
      entry = oi_malloc (sizeof (ListenerEntry));
      entry->name = oi_strdup (name);
      msgdc->list@list:append (entry);
    }
  return entry;
}

static void add (void *trait, const char *name, int id)
{
  Type *type = NULL;
  if (trait)
    type = *( (Type**)(trait));
  ListenerEntry *entry = self@msg_disconnect:get_entry_write (type, name);
  entry->type = type;
  entry->id = id;
}

@end

@trait Message
{
  Var  *callbacks;
};

typedef struct
{
  const char *message_name;
  void  (*callback) (Var *self, void *arg, void *user_data);
  void  *user_data;
} MessageEntry;

/*
int listen (Var          *oi_self,
            void         *trait_self,
            const char   *message_name,
            void        (*callback) (Var *self, void *arg, void *user_data),
            void         *user_data)
            */
int listen (Var          *oi_self,
            void         *trait_self,
            const char   *message_name,
            void         *callback,
            void         *user_data)
{
  Message *message = (Message*)trait_ensure (self, MESSAGE, NULL);
  MessageEntry *entry;
  entry = oi_malloc (sizeof (MessageEntry));
  entry->message_name = message_name;
  entry->callback = callback;
  entry->user_data = user_data;

  message->callbacks@list:append (entry);

  self@"oi:message-connect"((void*)message_name);

  /* add outselves to the disconnector trait, so that if the trait goes
   * away the message callback goes away.
   */
  if (oi_self)
    oi_self@msg_disconnect:add (trait_self, message_name, list_get_size (message->callbacks)-1);

  return (message->callbacks@list:get_size () - 1);
}

void handler_disconnect (int handler_id)
{
  Message *message = self@trait:get(MESSAGE);
  if (!message)
    return;
  self@"oi:message-disconnect"((void*)((MessageEntry*)list_get (message->callbacks, handler_id))->message_name);
  message->callbacks@list:remove_index (handler_id);
}

static void emit_matching! (void *entr, void *data)
{
  MessageEntry *entry = entr;
  void **emit_data = data;
  if (entr && entry->message_name && entry->callback &&
      !strcmp (emit_data[1], entry->message_name))
    {
      entry->callback (emit_data[0], emit_data[2], entry->user_data);
      emit_data[3]++; /* mark that we found one */
    }
}

void emit (const char *message_name,
           void       *arg)
{
  Message *message = self@trait:get(MESSAGE);
  if (message)//self@trait:check (MESSAGE))
    {
      void *emit_data[4] = {self, (void*)message_name, arg, NULL};
      list_each (message->callbacks, emit_matching, emit_data);
      if (emit_data[3] == NULL && strcmp(message_name, "message-trap"))
        {
          self@"message-trap"((void*)message_name);
        }
    }
}

static void free_sentry! (void *sentry)
{
  oi_free (sizeof (MessageEntry), sentry);
}
static void init ()
{
  message->callbacks = @list:new ();
  message->callbacks@list:set_destroy ((void*) free_sentry, NULL);
}
static void destroy ()
{
  message->callbacks@var:finalize();
}


static Var *queue = NULL;
static void dispatch_queue_thread! (void *data)
{
  while (1)
    {
      while (queue@list:get_size ())
        {
          Var *i;
          Var *oi;
          const char *message_name;
          void *arg;
          void (*closure) (void *arg);

          //oi_lock (queue);
          i = queue@list:get (0);
          oi =           i@oi:get_oi ("oi");
          message_name = i@oi:get_string ("message");
          arg =          i@oi:get_pointer ("arg");
          closure =      i@oi:get_pointer ("closure");
          oi@message:emit (message_name, arg);
          oi@ref:dec ();
          queue@list:remove_index (0);

          if (closure)
            closure (arg);
          //oi_unlock (queue);
        }
      usleep (1000);
    }
}
static Var *dispatch_queue! ()
{
  if (!queue)
    {
      pthread_t thread;
      queue = @list:new ();
      queue@mutex:lock();
      queue@list:set_destroy ((void*)ref_dec, NULL);
      queue@mutex:unlock();

      pthread_create (&thread, NULL, (void*)dispatch_queue_thread, NULL);
    }
  return queue;
}

/*
void emit_remote (const char *message_name,
                  void       *arg,
                  void (*closure) (void *arg))
*/
void emit_remote (const char *message_name,
                  void       *arg,
                  void       *closure)
{
  Var *item = var_new(NULL, NULL);
  dispatch_queue ()@mutex:lock ();
  item@oi:set_oi      ("oi", self);
  item@oi:set_string  ("message", message_name);
  item@oi:set_pointer ("arg", arg);
  item@oi:set_pointer ("closure", closure);
  dispatch_queue ()@list:append (item);
  dispatch_queue ()@mutex:unlock ();
}

static int
match_func! (void *entryp, void *callback)
{
  MessageEntry *entry = entryp;
  if (entry->callback == callback)
    return 1;
  return 0;
}

/* void handler_disconnect_by_func (void (*callback) (Var *self, void *arg, void *user_data))
 */

void handler_disconnect_by_func (void *callback)
{
  Message *message = self@trait:get(MESSAGE);
  int no;
  if (!message)
    return;
  no = message->callbacks@list:find_custom (match_func, callback);
  if (no>=0)
    self@message:handler_disconnect (no);
}

@end
