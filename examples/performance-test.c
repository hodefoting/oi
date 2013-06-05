#include <sys/time.h>
#include <stdio.h>
#include <glib.h>
#include "oi.h"

#define G_DISABLE_ASSERT
#define G_DISABLE_CHECKS
#define G_DISABLE_CAST_CHECKS
#define TEST_ITERATIONS 1000000

long oi_ticks (void);
static struct timeval start_time;
#define usecs(time) ((time.tv_sec - start_time.tv_sec) * 1000000 + time.tv_usec)

static inline void
init_ticks (void)
{
  static int done = 0;
  if (done)
    return;
  done = 1;
  gettimeofday (&start_time, NULL);
}

long oi_ticks (void)
{
  struct timeval measure_time;
  init_ticks ();
  gettimeofday (&measure_time, NULL);
  return usecs (measure_time) - usecs (start_time);
}


typedef struct
{
  Type   *trait_type;
  int     foo;
}  __attribute((packed))  Foo;

static void foo_init (Var oi, void *trait, Var args)
{
  Foo *foo = (Foo*)trait;
  foo->foo = 1;
}
OI(FOO, Foo, foo_init, NULL, NULL)

#define OI_FOO(oi) (trait_get_assert (oi, FOO))

float oi_get_foo (Var oi)
{
  Foo *foo = (Foo*)oi@trait:ensure (FOO, NULL);
  return foo->foo;
}

void  oi_set_foo (Var oi, float f)
{
  Foo *foo = (Foo*)oi@trait:get (FOO);
  foo->foo = f;
  oi@"notify"("foo");
}


#include <glib-object.h>

#define G_TYPE_OBJ            (g_obj_get_type ())
GType   g_obj_get_type        (void) G_GNUC_CONST;

#define G_OBJ(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_OBJ, GObj))
#define G_OBJ_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  G_TYPE_OBJ, GObjClass))
#define G_IS_OBJ(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_OBJ))
#define G_IS_OBJ_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  G_TYPE_OBJ))
#define G_OBJ_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  G_TYPE_OBJ, GObjClass))

typedef struct _GObj      GObj;
typedef struct _GObjClass GObjClass;
GObj *  g_obj_new  (void);
float   g_obj_get_foo (GObj *obj);
void    g_obj_set_foo (GObj *obj, float foo);

static void oops (void)
{
}

int perf_test (void)
{
  int i;
  int count = TEST_ITERATIONS;
  long start;

  float ref_unref_g = 0;
  float ref_unref_oi = 0;
  float condecon_g = 0;
  float condecon_oi = 0;
  float getsetaccesor_oi = 0;
  float getsetaccesor_c = 0;
  float getsetaccesor_g = 0;
  float getsetprop_oi = 0;
  float getsetprop_g = 0;
  float getsetpropnotify_oi = 0;
  float getsetpropnotify_g = 0;

  g_type_init ();
  if(1)oi_set_memory_functions (
      g_slice_free1, g_slice_alloc, NULL, g_strdup, g_free);

  {
    GObject *item = g_object_new (G_TYPE_OBJECT, NULL);
    start = oi_ticks ();
    for (i = 0; i < count; i ++)
      {
        g_object_ref (item);
        g_object_unref (item);
      }
    g_object_unref (item);
  }
  ref_unref_g = 1000000.0/((oi_ticks () - start)*1.0/count);

  {
  start = oi_ticks ();
  for (i = 0; i < count; i ++)
    {
      Var item = var_new (NULL, NULL);
      item@ref:dec();
    }
  condecon_oi = 1000000.0/((oi_ticks () - start)*1.0/count);

  start = oi_ticks ();
  for (i = 0; i < count; i ++)
    {
      GObject *item = g_object_new (G_TYPE_OBJECT, NULL);
      g_object_unref (item);
    }
  condecon_g = 1000000.0/((oi_ticks () - start)*1.0/count);

  {
    Var item = var_new (NULL, NULL);
    start = oi_ticks ();
    for (i = 0; i < count; i ++)
      {
        item@ref:inc ();
        item@ref:dec ();
      }
    ref_unref_oi = 1000000.0/((oi_ticks () - start)*1.0/count);
    item@ref:dec ();
  }


{
  Var item = var_new (NULL, NULL);
  start = oi_ticks ();
  for (i = 0; i < count; i ++)
    {
      float val = item@["a"float];
      item@["a"float]=val + 0.1;
    }
  getsetprop_oi = 1000000.0/((oi_ticks () - start)*1.0/count);
  item@ref:dec ();
}

{
  start = oi_ticks ();
      GObject *item = g_object_new (G_TYPE_OBJ, NULL);
  for (i = 0; i < count; i ++)
    {
      float f;
      g_object_get (item, "foo", &f, NULL);
      f++;
      g_object_set (item, "foo", f, NULL);
    }
  getsetprop_g = 1000000.0/((oi_ticks () - start)*1.0/count);
      g_object_unref (item);
}


{
  start = oi_ticks ();
      GObject *item = g_object_new (G_TYPE_OBJ, NULL);
  for (i = 0; i < count; i ++)
    {
      float f;
      f = g_obj_get_foo ((void*)item);
      f++;
      g_obj_set_foo ((void*)item, f);
    }
  getsetaccesor_c = 1000000.0/((oi_ticks () - start)*1.0/count);
      g_object_unref (item);
}


{
  start = oi_ticks ();
      GObject *item = g_object_new (G_TYPE_OBJ, NULL);
  for (i = 0; i < count; i ++)
    {
      float f;
      f = g_obj_get_foo (G_OBJ (item));
      f++;
      g_obj_set_foo (G_OBJ (item), f);
    }
  getsetaccesor_g = 1000000.0/((oi_ticks () - start)*1.0/count);
      g_object_unref (item);
}


{
  Var item = var_new (NULL, NULL);
  start = oi_ticks ();
  for (i = 0; i < count; i ++)
    {
      float f;
      f = item@oi:get_foo ();
      f++;
      item@oi:set_foo (f);
    }
  getsetaccesor_oi = 1000000.0/((oi_ticks () - start)*1.0/count);
  item@ref:dec ();
}


{
  Var item = var_new(NULL,NULL);
  start = oi_ticks ();
  item@message:listen (NULL, NULL, "notify", (void*)oops, NULL);
  for (i = 0; i < count; i ++)
    {
      float val = item@["a"float];
      item@["a"float]=val + 0.1;
    }
  getsetpropnotify_oi = 1000000.0/((oi_ticks () - start)*1.0/count);
  item@ref:dec ();
}


{
  start = oi_ticks ();
  GObject *item = g_object_new (G_TYPE_OBJ, NULL);
  g_signal_connect (item, "notify::foo", (void*)oops, NULL);
  for (i = 0; i < count; i ++)
    {
      float f;
      g_object_get (item, "foo", &f, NULL);
      f++;
      g_object_set (item, "foo", f, NULL);
    }
  getsetpropnotify_g = 1000000.0/((oi_ticks () - start)*1.0/count);
      g_object_unref (item);
}
}

   printf ("refunref: %2.0f%%\n", ref_unref_oi/ ref_unref_g * 100.0);
   printf ("condecon: %2.0f%%\n", condecon_oi / condecon_g * 100.0);
   printf ("getset_accesor_g: %2.0f%%\n", getsetaccesor_oi / getsetaccesor_g * 100.0);
   printf ("getset_prop: %2.0f%%\n", getsetprop_oi / getsetprop_g * 100.0);
   printf ("getset_propnotify: %2.0f%%\n", getsetpropnotify_oi / getsetpropnotify_g * 100.0);

   printf ("getset_accesor_c: %2.0f%%\n", getsetaccesor_oi / getsetaccesor_c * 100.0);

  return 0;
}

int main (int argc, char **argv)
{
  perf_test ();
  return 0;
}

struct _GObj
{
  GObject           parent_instance;
  float foo;
};

struct _GObjClass
{
  GObjectClass parent_class;
};



#include <string.h>

#include <glib-object.h>


enum
{
  PROP_0,
  PROP_FOO
};

static void      set_property (GObject      *gobject,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec);
static void      get_property (GObject    *gobject,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec);

G_DEFINE_TYPE (GObj, g_obj, G_TYPE_OBJECT)

static void
g_obj_init (GObj *object)
{
}

static void
g_obj_class_init (GObjClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = set_property;
  gobject_class->get_property = get_property;

  g_object_class_install_property (gobject_class, PROP_FOO,
                                   g_param_spec_float ("foo",
                                                        "Foo",
                                                        "just a float value...",
                                                        0.0, 255512345555.0, 0.0,
                                                        G_PARAM_READWRITE));
}

static void
set_property (GObject      *gobject,
              guint         property_id,
              const GValue *value,
              GParamSpec   *pspec)
{
  GObj *obj = G_OBJ (gobject);
  switch (property_id)
    {
      case PROP_FOO:
        obj->foo = g_value_get_float (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
        break;
    }
}

static void
get_property (GObject    *gobject,
              guint       property_id,
              GValue     *value,
              GParamSpec *pspec)
{
  GObj *obj = G_OBJ (gobject);
  switch (property_id)
    {
      case PROP_FOO:
        g_value_set_float (value, obj->foo);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, property_id, pspec);
        break;
    }
}

GObj *
g_obj_new (void)
{
  return g_object_new (G_TYPE_OBJ, NULL);
}

float g_obj_get_foo (GObj *obj)
{
  return obj->foo;
}
void g_obj_set_foo (GObj *obj, float foo)
{
  obj->foo = foo;
  g_object_notify (G_OBJECT (obj), "foo");
}
