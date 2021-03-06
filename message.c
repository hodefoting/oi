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

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "oi.h"
#include "pthread.h"

@generateheader

@trait Message
{
  var callbacks;
};


/* need an id-pool */
static long make_id! (void)
{
  static long id = 0;
  return ++id;
}

typedef struct
{
  int id;
  const char *message_name;
  void *(*callback) (var *self, void *arg, void *user_data);
  void  *user_data;
} PACKED MessageEntry;

/*
int listen (var          *oi_self,
            void         *trait_self,
            const char   *message_name,
            void        (*callback) (var *self, void *arg, void *user_data),
            void         *user_data)
            */
int listen (var           listener,
            void         *listener_trait,
            const char   *message_name,
            void         *callback,
            void         *user_data)
{
  MessageEntry *entry;
  entry = oi_malloc (sizeof (MessageEntry));
  entry->message_name = message_name;
  entry->callback = callback;
  entry->user_data = user_data;
  entry->id = make_id ();

  this->callbacks@list:append (entry);

  self@"oi:message-connect"((void*)message_name);

  /* add outselves to the disconnector trait, so that if the trait goes
   * away the message callback goes away.
   */
  if (listener)
    listener@own:add_message_cb (listener_trait, message_name, entry->id);

  return entry->id;
  //(this->callbacks@list:get_size () - 1);
}

static int match_id! (void *item, void *user_data)
{
  MessageEntry *entry = item;
  if (entry->id == (int)user_data)
    return 1;
  return 0;
}

void handler_disconnect (int handler_id)
{
  MessageEntry *entry;
  if (!this)
    return;
  int no = this->callbacks@list:find_custom (match_id, (void*)handler_id);
  entry = list_get (this->callbacks, no);
  if (entry)
  {
    self@"oi:message-disconnect"((void*)(entry->message_name));
    this->callbacks@list:remove_index (no);
  }
  else
  {
    fprintf (stderr, "uhm, missing handler %i\n", handler_id);
  }
}

static void emit_matching! (void *entr, void *data)
{
  MessageEntry *entry = entr;
  void **emit_data = data;
  if (emit_data[4])
    return;
  if (entr && entry->message_name && entry->callback &&
      !strcmp (emit_data[1], entry->message_name))
    {
      emit_data[4] = 
        entry->callback (emit_data[0], emit_data[2], entry->user_data);
      emit_data[3]++; /* mark that we found one */
    }
}

void *emit (const char *message_name,
            void       *arg)
{
  Message *message = self@trait:get(MESSAGE);
  /* short circuit if we do not have message trait */
  if (message)
    {
      void *emit_data[5] = {self, (void*)message_name, arg, NULL, NULL};
      list_each (message->callbacks, emit_matching, emit_data);
      if (emit_data[3] == NULL && strcmp(message_name, "message-trap"))
        {
          self@"message-trap"((void*)message_name);
        }
      return emit_data[4];
    }
  return NULL;
}

static void free_sentry! (void *sentry)
{
  oi_free (sizeof (MessageEntry), sentry);
}
static void init ()
{
  this->callbacks = @list:new ();
  this->callbacks@list:set_destroy ((void*) free_sentry, NULL);
}
static void destroy ()
{
  this->callbacks@var:finalize();
}


static var  queue = NULL;
static void dispatch_queue_thread! (void *data)
{
  while (1)
    {
      while (queue@list:get_size ())
        {
          var  i;
          var  oi;
          void *arg;
          void *(*cb)     (void *cb);
          void (*closure) (void *arg);

          i = queue@list:get (0);
          oi      =      i@["oi"oi];
          arg     =      i@["arg"pointer];
          cb      =      i@["cb"pointer];
          closure =      i@["closure"pointer];

          if (oi)
          {
          oi@message:emit (i@["message"string], arg);
          oi@ref:dec ();
          }
          else
          {
            assert (cb);
            /* ignoring retval */
            cb (arg);
          }
          queue@list:remove_index (0);

          if (closure)
            closure (arg);
        }
      usleep (1000); /* XXX: using locks here would be better.. */
    }
}
static var dispatch_queue! ()
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
  var item = var_new(NULL, NULL);
  dispatch_queue ()@mutex:lock ();
  item@property:set_oi      ("oi", self);
  item@property:set_string  ("message", message_name);
  item@property:set_pointer ("arg", arg);
  item@property:set_pointer ("closure", closure);
  dispatch_queue ()@list:append (item);
  dispatch_queue ()@mutex:unlock ();
}

void run_remote! (void *cb, void *arg, void *closure)
{
  var item = var_new(NULL, NULL);
  dispatch_queue ()@mutex:lock ();
  item@property:set_pointer ("cb", cb);
  item@property:set_pointer ("arg", arg);
  item@property:set_pointer ("closure", closure);
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

/* void handler_disconnect_by_func (void (*callback) (var *self, void *arg, void *user_data))
 */

void handler_disconnect_by_func (void *callback)
{
  int no;
  MessageEntry *entry;
  if (!this)
    return;
  no = this->callbacks@list:find_custom (match_func, callback);
  entry = list_get (this->callbacks, no);
  if (entry)
    {
      self@message:handler_disconnect (entry->id);
    }
}

@end
