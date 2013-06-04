/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!! GENERATED GENERATED GENERATED GENERATED GENERATED GENERATED   !!!! */
/* !!!!                                                               !!!! */
/* !!!! this file is generated from the corresponding .c file by oicc !!!! */
/* !!!!                                                               !!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#ifndef O_MUTEX_H
#define  O_MUTEX_H
extern OiType *MUTEX;
Oi *mutex_lock (Oi *self);
int mutex_trylock (Oi *self);
void mutex_unlock (Oi *self);
#endif
