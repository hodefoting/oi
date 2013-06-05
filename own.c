#include "oi.h"
@generateheader

@trait Own
{
  Var message_cbs;
  Var instances;
  Var custom;
};

typedef struct
{
  Var  instance;
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
  Own *own = self@trait:get_assert(OWN);
  Type *type = arg;
  int i;
again:
  for (i = 0; i < (own->message_cbs@list:get_size()); i++)
    {
      MessageEntry *entry = own->message_cbs@list:get(i);
      if (entry->type == type)
        {
          self@message:handler_disconnect (entry->id);
          own->message_cbs@list:remove(entry);
          goto again;
        }
    }

#if 0
    /* we are no longer needed, so remove ourself */
  if (own->message_cbs@list:get_size () == 0)
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
  own->instances = @list:new ();
  own->custom = @list:new ();
  own->message_cbs= @list:new ();

  own->instances@list:set_destroy ((void*)instance_entry_free, NULL);
  own->message_cbs@list:set_destroy ((void*)msg_entry_free, NULL);
}

static void destroy ()
{
  own->message_cbs@var:finalize();

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
  Own *own = ((void*)self@trait:ensure (OWN, NULL));
  int no;
  MessageEntry *entry;
  void *args[]={(void*)name, type};
  no = own->message_cbs@list:find_custom ((void*)listener_match_trait_and_name, args);
  if (no >= 0)
    entry = own->message_cbs@list:get (no);
  else
    {
      entry = oi_malloc (sizeof (MessageEntry));
      entry->name = oi_strdup (name);
      own->message_cbs@list:append (entry);
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

static void unref_instance_cb! (Var instance)
{
  instance@ref:dec();
}

void add_instance (Var *instance)
{
  /*
  Own *own = self@trait:ensure (OWN, NULL);
  InstanceEntry *entry = oi_malloc (sizeof(InstanceEntry));
  own->instances@list:append (entry); 
  entry->instance = instance;
  */;
  self@message:listen(self, NULL, "oi:die",
      unref_instance_cb, instance);
}

@end