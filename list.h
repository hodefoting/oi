#ifndef O_LIST_H
#define  O_LIST_H
void list_each (Oi *self, void *cbp, void *user_data);
void  * list_get (Oi *self, int no);
void list_remove_index_fast (Oi *self, int index);
void list_remove_index (Oi *self, int index);
void list_remove (Oi *self, void *data);
int list_get_size (Oi *self);
void list_list_remove_fast (Oi *self, void *data);
void list_remove_zombie_index_fast (Oi *self, int index);
void list_remove_zombie_index (Oi *self, int index);
void list_remove_zombie (Oi *self, void *data);
void list_remove_zombie_fast (Oi *self, void *data);
void list_set_destroy (Oi *self, void *destroy, void *user_data);
int list_find_custom (Oi *self, void *matchfunp,
                 void *user_data);
int list_find (Oi *self, void *data);
#define CS 8

void list_append (Oi *self, void *data);
extern OiType *LIST;
#endif
