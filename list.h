/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!! GENERATED GENERATED GENERATED GENERATED GENERATED GENERATED   !!!! */
/* !!!!                                                               !!!! */
/* !!!! this file is generated from the corresponding .c file by oicc !!!! */
/* !!!!                                                               !!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#ifndef _LIST_H_
#define  _LIST_H_
extern Type *LIST;
void list_each (Var *self, void *cbp, void *user_data);
void  * list_get (Var *self, int no);
void list_remove_index_fast (Var *self, int index);
void list_remove_index (Var *self, int index);
void list_remove (Var *self, void *data);
int list_get_size (Var *self);
void list_list_remove_fast (Var *self, void *data);
void list_remove_zombie_index_fast (Var *self, int index);
void list_remove_zombie_index (Var *self, int index);
void list_remove_zombie (Var *self, void *data);
void list_remove_zombie_fast (Var *self, void *data);
void list_set_destroy (Var *self, void *destroy, void *user_data);
int list_find_custom (Var *self, void *matchfunp,
                 void *user_data);
int list_find (Var *self, void *data);
void list_append (Var *self, void *data);
Var *list_new ();
#endif
