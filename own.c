/* Copyright (c) 2011, 2013 Øyvind Kolås
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
@generateheader

@trait Own
{
  var message_cbs;
  var instances;
  var custom;
};

typedef struct
{
  var   instance;
  Type *trait_type; /* or NULL/Trait if meaning the full instance */
} InstanceEntry;

typedef struct
{
  Type *type;
  char *name;
  int   id;
} MessageEntry;

"oi:remove-trait" (void *arg, void *user_data)
{
  Type *type = arg;
  int i;
again:
  for (i = 0; i < (this->message_cbs@list:get_size()); i++)
    {
      MessageEntry *entry = this->message_cbs@list:get(i);
      if (entry->type == type)
        {
          self@message:handler_disconnect (entry->id);
          this->message_cbs@list:remove(entry);
          goto again;
        }
    }

#if 0
    /* we are no longer needed, so remove ourself */
  if (this->message_cbs@list:get_size () == 0)
    self@trait:remove (OWN);
#endif
  return 0;
}

static void instance_entry_free! (InstanceEntry *entry)
{
  oi_free (sizeof (InstanceEntry), entry);
}

static void msg_entry_free! (MessageEntry *entry)
{
  if (entry->name)
    oi_strfree (entry->name);
  oi_free (sizeof (MessageEntry), entry);
}

static void init ()
{
  this->instances = @list:new ();
  this->custom = @list:new ();
  this->message_cbs= @list:new ();

  this->instances@list:set_destroy ((void*)instance_entry_free, NULL);
  this->message_cbs@list:set_destroy ((void*)msg_entry_free, NULL);
}

static void destroy ()
{
  this->message_cbs@var:finalize();

  self@message:handler_disconnect_by_func ((void*)own_oi_remove_trait_cb);
}

static int listener_match_trait_and_name! (void *listener_entry, void *arg)
{
  void **args=arg;
  MessageEntry *entry = listener_entry;
  return (entry->type == args[1] && !strcmp (entry->name, args[0]));
}

static MessageEntry *get_msg_entry_write (Type *type, const char *name)
{
  int no;
  MessageEntry *entry;
  void *args[]={(void*)name, type};
  no = this->message_cbs@list:find_custom ((void*)listener_match_trait_and_name, args);
  if (no >= 0)
    entry = this->message_cbs@list:get (no);
  else
    {
      entry = oi_malloc (sizeof (MessageEntry));
      entry->name = oi_strdup (name);
      this->message_cbs@list:append (entry);
    }
  return entry;
}

void add_message_cb (void *trait, const char *name, int id)
{
  Type *type = NULL;
  if (trait)
    type = *( (Type**)(trait));
  MessageEntry *entry = self@own:get_msg_entry_write (type, name);
  entry->type = type;
  entry->id = id;
}

static void unref_instance_cb! (var instance)
{
  instance@ref:dec();
}

void add_instance (var *instance)
{
  self@message:listen(self, NULL, "oi:die",
      unref_instance_cb, instance);
}

@end
