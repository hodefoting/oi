#ifndef O_MUTEX_H
#define  O_MUTEX_H
extern OiType *MUTEX;
Oi *mutex_lock (Oi *self);
int mutex_trylock (Oi *self);
void mutex_unlock (Oi *self);
#endif
