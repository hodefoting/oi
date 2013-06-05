/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!! GENERATED GENERATED GENERATED GENERATED GENERATED GENERATED   !!!! */
/* !!!!                                                               !!!! */
/* !!!! this file is generated from the corresponding .c file by oicc !!!! */
/* !!!!                                                               !!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#ifndef _OISTRING_H_
#define  _OISTRING_H_
extern Type *STRING;
Var string_clear (Var self);
Var string_appendc (Var self, int val);
Var string_append_str (Var self, const char *str);
Var string_append_string (Var self, Var oi2);
const char *string_get (Var self);
char *string_dissolve (Var self);
Var string_append_printf (Var self, const char *format, ...);
Var string_new (const char *initial);
#endif
