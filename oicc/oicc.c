#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>

char TMP_PATH[]="/tmp";
 
typedef struct State
{
  size_t len;

  char *header_name;

  FILE *fpr;
  FILE *fpw;

  char *line;

  char token[4096];
  size_t toklen;

  int state;

  char inbuf[409600];
  long ipos;

  int parend;
  int brackd;
  int atpos;

  char trait[256];
  char Trait[256];
  char TRAIT[256];

  char cmd[256];

  char init_int[40960];
  int init_int_pos;

  char init_pre[40960];
  int init_pre_pos;

  int in_trait;

  int got_init;
  int got_int_init;
  int got_pre_init;
  int got_destroy;
  int used_this;

  int gen_header;
  char header[40960];
  int  headpos;

  int  fun_start;
  int  in_fun;
  char *fun_name;
} State;

enum {
  S_NEUTRAL,
  S_GOT_SLASH,
  S_IN_COMMENT,
  S_IN_COMMENT_GOT_STAR,
  S_IN_AT,
  S_MAYBE_COMMA,
  S_MAYBE_EQUALS,
  S_ASSIGNMENT,
  S_IN_TRAIT_DEF,
  S_IN_END,
  S_MESSAGE,
  S_MESSAGE2,
};

#define FLUSH()  do{    fprintf (o->fpw, "%s", o->inbuf);o->ipos=o->inbuf[0]=0;;}while(0)
#if 0

#endif

#include <string.h>

void enter_function (State *o, const char *name)
{
  o->used_this = 0;
  o->fun_start = o->ipos;
  o->fun_name = strdup (name);
}

void leave_function (State *o)
{
  if (o->used_this && 
      o->fun_name &&
      strcmp (o->fun_name, "init") &&
      strcmp (o->fun_name, "destroy")
      )
  {
    int i;
    for (i = o->fun_start; i< o->ipos; i++)
      {
        if (o->inbuf[i]=='{')
          {
            int j;
            char insertion[512];
            sprintf (insertion, "{%s *this = trait_ensure (self, %s, NULL);",
                o->Trait, o->TRAIT);
            for (j = 0; insertion[j]; j++)
              {
                memmove ( &o->inbuf[i+1+j], &o->inbuf[i+j], o->ipos -(i+j));
                o->inbuf[i+j]=insertion[j];
                o->ipos++;
              }
            o->inbuf[i+j]=' ';
            break;
          }
      }
  }
  if (o->fun_name)
  {
    free (o->fun_name);
    o->fun_name = NULL;
  }
}

void handle_at (State *o)
{
  int j;
  int parencount = 0;
  int brackcount = 0;

  int got_data = 0;

  for (j = o->atpos-1; j>=0; j--)
    {
      switch (o->inbuf[j])
        {
          case ']':
            brackcount ++;
            goto bar;
            break;
          case '[':
            brackcount --;
            if(brackcount==0)
              got_data=0;
            goto bar;
            break;
          case ')':
            parencount++;
            if (parencount == 1 && o->atpos-j > 2)
              goto foo;
            break;
          case '(':
            if (parencount == 0 && o->atpos-j >2)
              goto foo;
            got_data = 0;
            parencount--;
            /* fallthrough */
          case ',':
          case '+':
          case '%':
          //case '>': /* cannot be here, since derferencing uses it..
          //check for -?  */
          case '<':
          case '!':
          case '}':
          case '{':

            /* - should be special
             * treatd when part of
             * -> construct */

          case '|':
          case '&':
          case '=':
          case '/':
          case '*':
bar:
          case ';':
            if (got_data && parencount == 0 && brackcount == 0)
              {
                int a;
                char preamble[512] = "";
                char tempbuf[2048];
                int offset = 1;
foo:
                while (o->inbuf[j+offset] == '\n' ||
                       o->inbuf[j+offset] == ' ') /* keep line numbering consistent for warnings and errors */
                  offset++;
                assert (o->inbuf[o->atpos] == '@');

                for (a = j+offset; a < o->atpos; a ++)
                  {
                    preamble[a-(j+offset)] = o->inbuf[a];
                  }
                preamble[a-(j+offset)] = 0;

                if (o->inbuf[o->atpos+1] == '[')
                {
                  char name[256] = "";
                  char type[256] = "";
                  int start;

                  for (a = o->atpos + 3; a < o->ipos; a ++)
                    {
                      if (o->inbuf[a]=='"')
                        break;
                      name[a-(o->atpos + 3)] = o->inbuf[a];
                    }
                  name[a-(o->atpos + 2)] = 0;
                  start = a +1;
                  for (a = start; a < o->ipos; a ++)
                    {
                      if (o->inbuf[a]==']')
                        break;
                      type[a-start] = o->inbuf[a];
                    }
                  type[a-start]=0;
                  sprintf (tempbuf, "property_get_%s (%s, \"%s\")", type, preamble, name);

                  o->state = S_MAYBE_EQUALS;
                }
                else
                {
                char trait[256] = "";
                char method[256] = "";

                for (a = o->atpos + 1; a < o->ipos; a ++)
                  {
                    /* do not do this splitting if we're a string.. */
                    if (trait[0]!='"' && o->inbuf[a]==':')
                      break;
                    trait[a-(o->atpos + 1)] = o->inbuf[a];
                  }

                trait[a-(o->atpos+1)]=0;

                int start;
                int end;
                start = a+1;
                for (a = start; a < o->ipos; a ++)
                  {
                    if (o->inbuf[a]=='(')
                      break;
                    if (o->inbuf[a]==']')
                      break;
                    method[a-start] = o->inbuf[a];
                  }
                end = a;
                method[end-start]=0;

                if(trait[0]=='"')
                  trait[strlen(trait)-1]=0;

                /* we should splice ourselves into the inbuf stream, and set a
                 * flag the we can be expecting to have to insert a , unless
                 * the next char is a )*/

                int foo = o->inbuf[a];
                o->state = S_MAYBE_COMMA;
                switch (o->inbuf[a])
                {
                  case '(':
                  sprintf (tempbuf, "%s_%s(%s", trait, method, preamble);
                  break;
                  case ']':
                  sprintf (tempbuf, "%s_%s::%s", trait, method, preamble);
                  break;
                  default:
                  if (trait[0]=='"')
                  {
                    if (o->token[0]=='(')
                    {
                      sprintf (tempbuf, "message_emit (%s, %s", preamble, trait);
                    }
                    else
                    {
                      sprintf (tempbuf, "string_new (%s)%c", trait, o->token[0]);
                      o->state = S_NEUTRAL;
                    }
                  }
                  break;
                }
              }
                  //fprintf (stderr, "%s\n", tempbuf);

                  int c = 0;
                  for (a = j+offset; tempbuf[c]; a++, c++)
                    {
                      o->inbuf[a] = tempbuf[c];
                    }
                  o->ipos = a;
                  o->inbuf[o->ipos]='\0';


                goto done;
              }
            break;
          default:
            got_data++;
            break;
        }
    }
done:
  ;

}

#include <ctype.h>

static void camel_to_upper (const char *camel, char *upper)
{
  int i;
  int COUNTER = 0;
  int was_upper = 1;
  for (i = 0; camel[i]; i++)
    {
     int u = toupper (camel[i]);
     if (isupper (camel[i]))
       {
         if (i != 0 &&
             (!was_upper || islower (camel[i+1])))
           {
             upper[COUNTER++] = '_';
           }
         was_upper = 1;
         upper[COUNTER++] = u;
       }
     else
       {
         was_upper = 0;
         upper[COUNTER++] = u;
       }
    }
  upper[COUNTER] = 0;
}

static void camel_to_lower (const char *camel, char *lower)
{
  int i;
  int counter = 0;
  int was_upper = 1;
  for (i = 0; camel[i]; i++)
    {
      int l = tolower (camel[i]);
      if (isupper (camel[i]))
        {
          if (i != 0 &&
              (!was_upper || islower (camel[i+1])))
            {
              lower[counter++] = '_';
            }
          was_upper = 1;
          lower[counter++] = l;
        }
      else
        {
          was_upper = 0;
          lower[counter++] = l;
        }
    }
  lower[counter] = 0;
}


void process_token (State *o)
{
  int do_flush = 0;
  
  if (o->toklen == 0)
    return;
  if (!strcmp ("this", o->token))
    o->used_this++;

  if (o->state == S_IN_END)
    {
      if (o->token[0] == ' '||
          o->token[0] == '\n' ||
          o->token[0] == '\t' ||
          o->token[0] == '/' ||
          o->token[0] == ',')
        {

          int add_nls = 0;
          {
            int i;
            for (i = o->atpos; i < o->ipos; i++)
              if (o->inbuf[i] == '\n')
                add_nls++;
          }
          add_nls++;
          o->ipos = o->atpos;
          {
            char buf[1024];
            int pos = 0;
            int i;

            if (o->init_pre_pos)
              {
                o->init_pre[o->init_pre_pos]=0;

            pos += sprintf (&buf[pos],
                "static void %s_init_pre (var self){%s}", o->trait, o->init_pre);
                o->init_pre_pos = 0;
              }

            if (o->init_int_pos)
              {
                o->init_int[o->init_int_pos]=0;

            pos += sprintf (&buf[pos], "static void %s_init_int (var self){"
  "%s *%s=trait_get(self,%s);%s}", o->trait, o->Trait, o->trait, o->TRAIT, o->init_int);
                o->init_int_pos = 0;
              }

            pos += sprintf (&buf[pos], "OI(%s, %s,", o->TRAIT, o->Trait);

            if (o->got_init)
              pos += sprintf (&buf[pos], "(void*)%s_init, ", o->trait);
            else
              pos += sprintf (&buf[pos], "NULL, ");

            if (o->got_pre_init)
              pos += sprintf (&buf[pos], "(void*)%s_init_pre, ", o->trait);
            else
              pos += sprintf (&buf[pos], "NULL, ");

            if (o->got_int_init)
              pos += sprintf (&buf[pos], "(void*)%s_init_int, ", o->trait);
            else
              pos += sprintf (&buf[pos], "NULL, ");

            if (o->got_destroy)
              pos += sprintf (&buf[pos], "(void*)%s_destroy);", o->trait);
            else
              pos += sprintf (&buf[pos], "NULL);");

            for (i = 0; buf[i]; i++)
              o->inbuf[o->ipos++] = buf[i];
            for (i = 0; i < add_nls; i++)
              o->inbuf[o->ipos++] = '\n';
            o->inbuf[o->ipos] = 0;
          }

          o->state = S_NEUTRAL;
          o->in_trait = 0;
          o->got_init = 0;
          o->got_int_init = 0;
          o->got_pre_init = 0;
          o->got_destroy = 0;

          return;

        }
      else
        {
          int i;
          for (i = 0; i < o->toklen; i++)
            {
              o->in_trait++;
              if (o->in_trait > 6)
                {
                  o->Trait[o->in_trait-7] = o->token[i];
                }
            }
          o->Trait[o->in_trait] = 0;
        }
    }
  if (o->state == S_IN_TRAIT_DEF)
    {
      char dependencies[256]="";
      int deppos=0;

      if (o->token[0] == '{')
        {
          char *deps = NULL;
          int i;
          o->state = S_NEUTRAL;
          o->brackd++; /* XXX: since we return */

          while (
              o->Trait[strlen(o->Trait)-1] == '\n' ||
              o->Trait[strlen(o->Trait)-1] == ' ')
            o->Trait[strlen(o->Trait)-1] = 0;

          if (strchr (o->Trait, '<'))
            {
              deps = strdup (strchr (o->Trait, '<')+1);
              *strchr (o->Trait, '<')=0;

              o->got_pre_init ++;
            }

          while (
              o->Trait[strlen(o->Trait)-1] == '\n' ||
              o->Trait[strlen(o->Trait)-1] == ' ')
            o->Trait[strlen(o->Trait)-1] = 0;

          camel_to_upper (o->Trait, o->TRAIT);
          camel_to_lower (o->Trait, o->trait);

          if (deps)
          {
            while (*deps == ' ' || *deps == '\n')deps++;
            char dep[256];
            int depp = 0;
            int i;
            for (i = 0; deps[i]; i++)
              {
                switch (deps[i])
                  {
                    case ' ':
                      {
                      char upper[256];
                      dep[depp]=0;
                      camel_to_upper (dep, upper);
             o->init_pre_pos += sprintf (&o->init_pre[o->init_pre_pos],
              "trait_ensure (self, %s, NULL);", upper);
                      depp=0;
                      }
                      break;
                    default:
                      dep[depp++]=deps[i];
                      break;
                  }
              }
            if (depp)
            {
             char upper[256];
             dep[depp]=0;
             camel_to_upper (dep, upper);
             o->init_pre_pos += sprintf (&o->init_pre[o->init_pre_pos],
              "trait_ensure (self, %s, NULL);", upper);
             o->got_pre_init ++;
            }

          }

          int add_nls = 0;
          {
            int i;
            for (i = o->atpos; i < o->ipos; i++)
              if (o->inbuf[i] == '\n')
                add_nls++;
          }
          o->ipos = o->atpos;
          {
            char buf[1024];
            int i;

            if (!memcmp (o->cmd, "main", 4))
            {
            sprintf (buf, "int main (int argc, char **argv){var self=var_new(PROGRAM, argv);var __attribute__((__unused__)) args=program_get_args(self);");
            o->in_trait=0;
            }
            else if (add_nls)
            {
            if (o->gen_header)
            o->headpos += sprintf (&o->header[o->headpos],
                "extern Type *%s;\n", o->TRAIT);
            sprintf (buf, "extern Type *%s;typedef struct _%s %s;\nstruct _%s{Type *trait_type;",
                o->TRAIT, o->Trait, o->Trait, o->Trait);
            add_nls--;
            }
            else
            {
            if (o->gen_header)
            o->headpos += sprintf (&o->header[o->headpos],
                "extern Type *%s;\n", o->TRAIT);
            sprintf (buf, "extern Type *%s;typedef struct _%s %s;struct _%s{varTrait trait;",
                o->TRAIT, o->Trait, o->Trait, o->Trait);
            }
            for (i = 0; buf[i]; i++)
              {
                o->inbuf[o->ipos++] = buf[i];
              }
            for (i = 0; i < add_nls; i++)
              {
                o->inbuf[o->ipos++] = '\n';
              }
            o->inbuf[o->ipos] = 0;
          }

          return;

        }
      else
        {
          /* should keep the command,. and only store the trait name
           * when we know it was a trait - this initial implemntation
           * must go..
           */

          int i;
          for (i = 0; i < o->toklen; i++)
            {
              o->in_trait++;
              if (o->in_trait > 6)
                {
                  o->Trait[o->in_trait-7] = o->token[i];
                }
              else if (o->in_trait < 6)
                {
                  o->cmd[o->in_trait-1] = o->token[i];
                }
            }
          o->Trait[o->in_trait-6] = 0;

          if (!memcmp (o->cmd, "gene", 4))
          {
            char buf[256];
            sprintf(buf, "#include \"%s\"", o->header_name);
            o->gen_header=1;
            o->in_trait=0;

            o->state = S_NEUTRAL;

            int add_nls = 0;
            {
              int i;
              for (i = o->atpos; i < o->ipos; i++)
                if (o->inbuf[i] == '\n')
                  add_nls++;
          }

          o->ipos = o->atpos;
            for (i = 0; buf[i]; i++)
              {
                o->inbuf[o->ipos++] = buf[i];
              }
            for (i = 0; i < add_nls; i++)
              {
                o->inbuf[o->ipos++] = '\n';
              }
            o->inbuf[o->ipos] = 0;
            return;
          }

        }
    }

  if (o->state == S_ASSIGNMENT)
  {
    {
      int j;
      if (o->token[0] == ';')
        {
          int i;
          for (i = o->ipos;
              !(o->inbuf[i-2]=='t' &&
                o->inbuf[i-1]=='y' &&
                o->inbuf[i+0]=='_' &&
                o->inbuf[i+1]=='g' &&
                o->inbuf[i+2]=='e' &&
                o->inbuf[i+3]=='t' &&
                o->inbuf[i+4]=='_'); i--);
          o->inbuf[i + 1] = 's';

          for (i = o->ipos;
              !(o->inbuf[i+0]=='"' &&
                o->inbuf[i+1]==')' &&
                o->inbuf[i+2]=='='); i--);
          o->inbuf[i + 1] = ',';
          o->inbuf[i + 2] = ' ';

          o->state = S_NEUTRAL;
          o->inbuf[o->ipos++] = ')';
        }
      for (j = 0; j < o->toklen; j++)
        o->inbuf[o->ipos++] = o->token[j];
      o->inbuf[o->ipos]=0;
      return;
    }
  }
  else if (o->state == S_MAYBE_EQUALS)
  {
    if (o->token[0] == '=')
      {
        o->state = S_ASSIGNMENT;
      }
    else
      {
        o->state = S_NEUTRAL;
      }
  }
  else if (o->state == S_MAYBE_COMMA)
    {
      if (o->token[0] == ')')
      {
        o->state = S_NEUTRAL;
      }
      else if (o->token[0] == ' ')
      {
        o->state = S_MAYBE_COMMA;
      }
      else
      {
        o->inbuf[o->ipos++] = ',';
        o->inbuf[o->ipos++] = ' ';
        o->inbuf[o->ipos] = '\0';
        o->state = S_NEUTRAL;
      }
    }
  if (o->state == S_IN_AT)
    {
      //if (o->toklen == 1)
        {
          switch (o->token[0])
            {
              case ']':
              case '(':
              case ';':
              case ',':
              //case '\n':
                /* manually insert since we're bailing of treatment later.. */
                o->inbuf[o->ipos++] = o->token[0];
                o->inbuf[o->ipos]=0;

                handle_at (o);
                return;
                break;
            }
        }
      //else
        {
        }
    }

  //if (o->toklen == 1)
  {
    switch (o->state)
      {
        case S_NEUTRAL:
           switch (o->token[0])
             {
               case '@':
                 /* figure out how far into past to go */

                 if (o->brackd)
                   {
                     o->state = S_IN_AT;
                     o->atpos = o->ipos;
                   }
                 else
                   {
                     if (o->in_trait)
                       {
                         o->state = S_IN_END;
                         o->atpos = o->ipos;
                       }
                     else
                       {
                         o->state = S_IN_TRAIT_DEF;
                         o->atpos = o->ipos;
                       }
                   }
                 break;

               case '{':
                 o->brackd++;
                 break;
               case '}':
                 o->brackd--;
                 if (o->brackd == 0 && o->in_fun >= 0)
                   {
                     o->in_fun = 0;
                     leave_function (o);
                   }
                 break;
               case ';':
                 o->parend = 0;
                 //FLUSH();
                 break;

               case '(':
                 /* knowing if this si the one is hard.. given function
                  * pointers and other possiblities?
                  */
                 o->parend++;

                 if (o->brackd==0 && o->in_trait && o->parend==1)
                   {
                     int pos = o->ipos;
                     char name[1024];
                     char tempbuf[2045];
                     int i;

                     pos--;
                     while (o->inbuf[pos]==' ' ||
                            o->inbuf[pos]=='\n' ||
                            o->inbuf[pos]=='\t')
                       pos--;

                     if (o->inbuf[pos]=='!')
                       {
                         o->inbuf[pos]=' ';
                         break;
                       }

                     if (o->inbuf[pos+1]==' ' ||
                         o->inbuf[pos+1]=='\n' ||
                         o->inbuf[pos+1]=='\t')
                     o->inbuf[pos+1]=0;

                     while (o->inbuf[pos]!=' ' &&
                            o->inbuf[pos]!='*' &&
                            o->inbuf[pos]!='\n' &&
                            o->inbuf[pos]!='\t')
                       pos--;
                     pos++;
                     for (i = 0; pos + i < o->ipos; i++)
                       name[i] = o->inbuf[pos + i];
                     name[i] = 0;

                     enter_function (o, name);

                     if (!strcmp (name, "init"))
                       {
                         o->got_init ++;
                       }
                     if (!strcmp (name, "destroy"))
                       {
                         o->got_destroy ++;
                       }

                     if (name[0]=='"')
                       {
                         int i;
                         char *cbname;
                         /* XXX: transform ' ' '-' etc.. */
                         memcpy(name, &name[1], sizeof(name)-1);
                         name[strlen(name)-1]=0;
                         cbname = strdup (name);
                         for (i = 0; cbname[i]; i++)
                           {
                             if (cbname[i]==':' ||
                                 cbname[i]==' ' ||
                                 cbname[i]=='-' ||
                                 cbname[i]==';')
                                cbname[i]='_';
                           }


                         sprintf (tempbuf, "static int %s_%s_cb (var self", o->trait, cbname);

                         /* add to list being built up for contents
                          * of init_int implementation. 
                          */
                         o->got_int_init++;

                         o->init_int_pos += sprintf (&o->init_int[o->init_int_pos],
                             "message_listen(self, (void*)self,(void*)%s,\"%s\", (void*)%s_%s_cb, %s);",
                             o->trait, name, o->trait, cbname, o->trait);
                       }
                     else
                     if (!strcmp (name, "new"))
                     {
                       sprintf (tempbuf, "%s_%s (", o->trait, name);
                     }
                     else if (!strcmp (name, "init") ||
                         !strcmp (name, "destroy"))
                     {
                     sprintf (tempbuf, "%s_%s (var self, %s *this", o->trait, name, o->Trait);
                     }
                     else
                     sprintf (tempbuf, "%s_%s (var self", o->trait, name);

                     for (i = 0; tempbuf[i]; i++)
                       o->inbuf[pos++] = tempbuf[i];
                     o->inbuf[pos] = 0;
                     o->ipos = pos;
                     o->state = S_MAYBE_COMMA;

                     if (!strcmp (name, "new"))
                       o->state = S_NEUTRAL;
                     return;
                   }
                 break;
               case ')':
                 if (o->brackd==0 && o->parend==1 && o->gen_header)
                   {
                     int start = 0;
                     int done = 0;
                     for (start = 0; !done && o->ipos+start > 0 ; start--)
                       {
                         switch (o->inbuf[o->ipos+start])
                           {
                             case ';':
                             case '}':
                             case '/':
                             case '{':
                               done = 1;
                             break;
                             default:
                              break;
                           }
                       }
                     start++;
                     while (o->inbuf[o->ipos+start] == ' '||
                            o->inbuf[o->ipos+start] == '\n'||
                            o->inbuf[o->ipos+start] == '}' ||
                            o->inbuf[o->ipos+start] == '/'
                            )
                       start++;
                     if (o->in_trait)
                     {
                       if (!strstr (&o->inbuf[o->ipos+start], "static"))
                       {
                         o->headpos += sprintf (&o->header[o->headpos], "%s);\n", &o->inbuf[o->ipos+start]);
                       }
                     }

                     o->in_fun++;
                   }
                 break;
                 o->parend--;
                 break;

               case '/':
                 o->state = S_GOT_SLASH;
                 break;
               default:
                 break;
             }
          break;
        case S_GOT_SLASH:
           switch (o->token[0])
             {
               case '*':
                 o->state = S_IN_COMMENT;
                 //FLUSH();
                 break;
               default:
                 o->state = S_NEUTRAL;
                 break;
             }
          break;
        case S_IN_COMMENT:
           switch (o->token[0])
             {
               case '*':
                 o->state = S_IN_COMMENT_GOT_STAR;
                 break;
               default:
                 break;
             }
          break;
        case S_IN_COMMENT_GOT_STAR:
           switch (o->token[0])
             {
               case '/':
                 o->state = S_NEUTRAL;
                 do_flush++;
                 break;
               default:
                 o->state = S_IN_COMMENT;
                 break;
             }
          break;
      }
  }
  
  char *at = strchr (o->token, '@');
  char *dot = strchr (o->token, '.');

#if 0
  if (at && dot)
  {
    const char *klass = at+1;
    const char *method = dot+1;
    *at = 0;
    *dot = 0;

    fprintf (o->fpw, "%s_%s %s", klass, method, o->token);
    printf ("<<<%s_%s %s>>>", klass, method, o->token);
  }
  else
  {
    fprintf (o->fpw, "%s", o->token);
    printf ("%s", o->token);
  }
#endif

  {
    int j;
    for (j = 0; j < o->toklen; j++)
      {
        o->inbuf[o->ipos++] = o->token[j];
#if 0
        if (o->brackd == 0)
          printf ("%c", o->token[j]);
#endif
      }
    o->inbuf[o->ipos]=0;
  }

  //if (do_flush)
  //  FLUSH();
    //fprintf (o->fpw, "%s", o->token);
    //printf ("%s", o->token);
}

void process_line (State *o)
{
  char *p = o->line;
  int i;
#if 0
  o->toklen = 0;
  o->token[o->toklen] = 0;
#endif
  for (p = o->line; *p; p++)
    {
      switch (*p)
        {
          case ' ':
          case '-':
          case '*':
          case '!':
          case '@':
          case '.':
          case '[':
          case ']':
          case '{':
          case '#':
          case '>':
          case '<':
          case '}':
          case ')':
          case '(':
          case '&':
          case '/':
          case '"':
          case '\'':
          case '\\':
          case '+':
          case ',':
          case ';':
          case '\t':
          case '\n':
            process_token (o);
            o->token[0] = o->toklen = 0;
            {
              /* this was the reason for repeated ))s.. and higher toklen.. */
              char self;
              for (self = *p; *p == self && o->toklen == 0; p++)
                {
                  o->token[o->toklen++] = *p;
                  o->token[o->toklen] = 0;
                }
              p--;
            }
            process_token (o);
            o->token[0] = o->toklen = 0;
            break;
          default:
            o->token[o->toklen++] = *p;
            o->token[o->toklen] = 0;
            break;
        }
    }

#if 0
  if (o->toklen)
  {
    process_token (o);
    o->toklen = 0;
    o->token[o->toklen] = 0;
  }
#endif
}

int oicc_filter (FILE *fpr, FILE *fpw, const char *header_name)
{
  State state = {0.};
  State *o = &state;
  ssize_t read;

  o->header_name = strdup (header_name);

  //unlink (dst_file);
  state.fpr = fpr;
  if (state.fpr == NULL)
    return -1;

  state.len = 0;
  state.line = NULL;

  state.fpw = fpw;
  if (state.fpw == NULL)
    return -2;
  while ((read = getline (&state.line, &state.len, state.fpr)) != -1)
    {
      process_line (&state);
    }
  FLUSH();

  if (state.toklen)
    process_token (&state);

  if (state.line)
    free (state.line);

  if (state.gen_header && state.headpos)
    {
      char header[40960];
      {
        char *flattened = strdup (header_name);
        int i;
        for (i = 0; flattened[i];i++)
        {
          switch (flattened[i])
          {
            case '.': flattened[i] = '_';
            case '-': flattened[i] = '_';
            default:flattened[i] = toupper(flattened[i]);
          }
        }
        sprintf (header,
"/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */\n"
"/* !!!! GENERATED GENERATED GENERATED GENERATED GENERATED GENERATED   !!!! */\n"
"/* !!!!                                                               !!!! */\n"
"/* !!!! this file is generated from the corresponding .c file by oicc !!!! */\n"
"/* !!!!                                                               !!!! */\n"
"/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */\n"
"#ifndef _%s_\n"
"#define  _%s_\n"
"%s"
"#endif\n", flattened, flattened, state.header);
        free (flattened);
      
        FILE * hf = fopen (header_name, "r");
        if (hf)
          {
            fread (state.header, 1, sizeof (state.header), hf);
            fclose (hf);
          }

        if (hf == NULL || strcmp (state.header, header))
          {
            hf = fopen (header_name, "w");
            fprintf (hf, "%s", header);
            fclose (hf);
          }
      }

    }

  return 0;
}

char *mappings[64][2];
int   map_count;

static void output_linebuf (char *string)
{
  int i;
  for (i = 0; i < map_count; i++)
  {
    int len = strlen (mappings[i][1]);
    if (!memcmp (mappings[i][1], string, len))
      {
        printf ("%s", mappings[i][0]);
        printf ("%s", &string[len]);
        return;
      }
  }
  printf ("%s", string);
}

/* launch gcc, with filtering of it's error messages to redirect us
 * to the source source of the error.
 */
int filtered_cc (char commandline[])
{
  char commandline2[512];
  char linebuf[2048];
  char lpos = 0;
  FILE *pipe;
  int  count;
  char inchar;

  sprintf (commandline2, "%s 2>&1", commandline);
  pipe = popen(commandline2,"r");
  if (pipe == NULL)
     {
       fprintf(stderr ,"command not found?: %s\n",commandline2);
       return -23;
     }

  for (inchar = fgetc(pipe), count = 0; 
      (!feof(pipe) && inchar != 0xffffffff);
       inchar = fgetc(pipe), count ++)
     {
       switch (inchar)
         {
           case '\n':
             linebuf[lpos++] = inchar;
             linebuf[lpos] = 0;
             output_linebuf (linebuf);
             lpos = 0;
             linebuf[lpos] = 0;
             break;
           default:
             linebuf[lpos++] = inchar;
             linebuf[lpos] = 0;
         }
     }
  if (lpos)
    output_linebuf (linebuf);
  lpos = 0;
  exit (pclose(pipe));
}

static int oicc_main (int argc, char **argv)
{
  int i;             
  mkdir (TMP_PATH, 0777);
  for (i = 0; i < argc; i++)
    {
      int len = strlen (argv[i]);
      if (len > 3)
        {
          if (argv[i][len-2] == '.' &&
              argv[i][len-1] == 'c' &&

              /* in case we're filtering .c file to .c.c files ... */
              !(argv[i][len-4] == '.' &&
                argv[i][len-3] == 'c' )
              )
           {
             char *replacement = malloc (len + 50);
             char *header_name = strdup(basename (argv[i]));
             header_name[strlen(header_name)-1]='h';

             sprintf (replacement, "%s/%s", TMP_PATH, argv[i]);
             int j;
             /* make subdirs be prefix of filename */
             for (j = strlen(TMP_PATH)+1; replacement[j]; j++)
               {
                 char r = replacement[j];
                 if (r == '/')
                   replacement[j]='_';
               }

             FILE *fpr, *fpw;
             fpr = fopen (argv[i], "r");
             fpw = fopen (replacement, "w");
             oicc_filter (fpr, fpw, header_name);
             fclose (fpr);
             fclose (fpw);
             mappings[map_count][0] = argv[i];
             mappings[map_count][1] = replacement;
             map_count++;
             argv[i] = replacement;
           }
        }
    }

  argv[0]="gcc";
  {
    int i;
    char cmd[4096];
    int pos = 0;
    pos += sprintf (&cmd[pos], "gcc ");

    for (i = 1; i < argc; i ++)
      {
        int j;
        pos += sprintf (&cmd[pos], " ");
        for (j = 0; argv[i][j]; j++)
          {
            switch (argv[i][j])
              {
                case '"':
                case '\\':
                case ' ':
                  cmd[pos++] = '\\';
                default:
                cmd[pos++] = argv[i][j];
                break;
              }
          }
      }
    cmd[pos]=0;

    return filtered_cc (cmd);
  }
}

static int oipp_main (int argc, char **argv)
{
  int i;
  if (argc == 1)
    goto help;
  for (i = 1; i < argc; i++)
    {
      int len = strlen (argv[i]);
     
      if (!strcmp (argv[i], "--help"))
        {
help:
          printf ("Reads in oi source and spits out equivalent C to stdout\n");
          return 0;
        }
      
         if (argv[i][0]!='-')
           {
             char *header_name = strdup(basename (argv[i]));
             header_name[strlen(header_name)-1]='h';

             FILE *fpr;
             fpr = fopen (argv[i], "r");
             oicc_filter (fpr, stdout, header_name);
             fclose (fpr);
           }
    }
  return 0;
}

int main (int argc, char **argv)
{
  const char *task = basename (argv[0]);
  if (!strcmp (task, "oicc"))
    return oicc_main (argc, argv);
  if (!strcmp (task, "oipp"))
    return oipp_main (argc, argv);

  printf ("Error: unknown oicc task %s\n", task);
  return -1;
}
