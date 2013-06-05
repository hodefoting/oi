/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!! GENERATED GENERATED GENERATED GENERATED GENERATED GENERATED   !!!! */
/* !!!!                                                               !!!! */
/* !!!! this file is generated from the corresponding .c file by oicc !!!! */
/* !!!!                                                               !!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#ifndef _MESSAGE_H_
#define  _MESSAGE_H_
extern Type *MESSAGE;
int message_listen (Var *self, Var          *listener,
            void         *listener_trait,
            const char   *message_name,
            void         *callback,
            void         *user_data);
void message_handler_disconnect (Var *self, int handler_id);
void message_emit (Var *self, const char *message_name,
           void       *arg);
void message_emit_remote (Var *self, const char *message_name,
                  void       *arg,
                  void       *closure);
void message_handler_disconnect_by_func (Var *self, void *callback);
#endif
