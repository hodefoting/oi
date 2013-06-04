/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!! GENERATED GENERATED GENERATED GENERATED GENERATED GENERATED   !!!! */
/* !!!!                                                               !!!! */
/* !!!! this file is generated from the corresponding .c file by oicc !!!! */
/* !!!!                                                               !!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#ifndef O_MUTEX_H
#define  O_MUTEX_H
extern Type *MUTEX;
Var *mutex_lock (Var *self);
int mutex_trylock (Var *self);
void mutex_unlock (Var *self);
#endif
