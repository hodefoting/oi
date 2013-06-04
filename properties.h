/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!! GENERATED GENERATED GENERATED GENERATED GENERATED GENERATED   !!!! */
/* !!!!                                                               !!!! */
/* !!!! this file is generated from the corresponding .c file by oicc !!!! */
/* !!!!                                                               !!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#ifndef _PROPERTIES_H_
#define  _PROPERTIES_H_
extern Type *PROPERTY;
void property_each (Var *self, void *cb, void *user_data);
void   oi_set_float  (Var *oi, const char *name, float       value);
void   oi_set_int  (Var *oi, const char *name, int         value);
void   oi_set_string  (Var *oi, const char *name, const char *value);
void   oi_set_oi  (Var *oi, const char *name, Var *value);
void   oi_set_pointer  (Var *oi, const char *name, void       *value);
float  oi_get_float  (Var *oi, const char *name);
int    oi_get_int  (Var *oi, const char *name);
const char *oi_get_string  (Var *oi, const char *name);
Var *oi_get_oi  (Var *oi, const char *name);
void  *oi_get_pointer      (Var *oi, const char *name);
int property_is_string (Var *self, const char *name);
#endif
