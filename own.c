#include "oi.h"
@generateheader

@trait Own
{
  Var *message_cbs;
  Var *instances;
  Var *custom;
};

typedef struct
{
  Var  *instance;

  Type *type; /* or NULL/Trait if meaning the full instance */
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

static void lnrfree! (MessageEntry *entry)
{
  if (entry->name)
    oi_strfree (entry->name);
  oi_free (sizeof (MessageEntry), entry);
}

static void init ()
{
  own->message_cbs= @list:new ();
  own->message_cbs@list:set_destroy ((void*)lnrfree, NULL);
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

static MessageEntry *get_entry_write (Type *type, const char *name)
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
  MessageEntry *entry = self@own:get_entry_write (type, name);
  entry->type = type;
  entry->id = id;
}

@end
