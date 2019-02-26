/* No memory management here, since help is a dead op */

#include "commands.h"
#include "conf.h"
#include "common.h"

#include <string.h>

#ifndef BOLD
#define BOLD        "\033[1m"
#endif

#ifndef BOLD_END
#define BOLD_END    "\033[0m"
#endif

#ifndef ITALIC
#define ITALIC      "\033[4m"
#endif

#ifndef ITALIC_END
#define ITALIC_END  "\033[0m"
#endif

#define P(...) \
   printf(__VA_ARGS__)

#define PA(FMT, ...) \
   printf(fill_attrs(FMT), ##__VA_ARGS__)

#define ATTRS(...) \
   fill_attrs(__VA_ARGS__)

inline __attribute__((always_inline))
char* firstline(const char *s) {
   return strndup(s, strcspn(s, "\n"));
}

static
const char *attrs[] = {
   BOLD,
   BOLD_END,
   ITALIC,
   ITALIC_END
};

static void  help_all();
static void  help_keys();
static void  help_conf();
static void  help_command();
static void  help_commands();

static char* fill_attrs(const char *);
static char* indent(const char *, int);

int help(const char *prog, const char *usage, const char *topic) {
   command_t *cmd;

   if (! topic)
      P(usage, prog, prog, prog, prog, prog);
   else if ((cmd = get_command(topic)))
      help_command(cmd);
   else if ((cmd = (command_t*) get_conf_command(topic)))
      help_command(cmd);
   else if (streq(topic, "all"))
      help_all();
   else if (streq(topic, "keys"))
      help_keys();
   else if (strprefix("config", topic))
      help_conf();
   else if (strprefix("commands", topic))
      help_commands();
   else
      P(usage, prog, prog, prog, prog, prog);

   return 0;
}

static void help_all() {
   help_conf();
   P("\n");
   help_commands();
   P("\n");
   help_keys();
}

static void help_commands() {
   PA("_Available commands_\n\n");
   for (int i = 0; i < commands_size; ++i)
      PA("*%-15s* %s\n", commands[i]->name, firstline(commands[i]->desc));
}

static void help_conf() {
   PA("_Configuration keywords_\n\n");
   for (int i = 0; i < conf_commands_size; ++i)
      PA("*%-15s* %s\n", conf_commands[i]->name, firstline(conf_commands[i]->desc));
}

static void help_command(command_t *cmd) {
   PA("*%s*", cmd->name);

   if (cmd->opts)
      P(" [OPTIONS]");

   if (cmd->args) {
      for (const char **arg = cmd->args; *arg; ++arg)
         if (**arg == '+')
            PA(" _%s_ ...", *arg + 1);
         else
            PA(" _%s_ ",    *arg);
   }

   P("\n");
   P(indent(ATTRS(cmd->desc), 3));
   P("\n");

   if (cmd->opts) {
      for (const command_opt_t *opt = cmd->opts; opt->opt; ++opt) {
         PA("     *-%c*", opt->opt);

         if (opt->meta) {
            PA(" _%s_", opt->meta);
            for (int i = 10 - strlen(opt->meta) - 1; i--; P(" "));
         }
         else
            P("%-10s", "");

         PA("%s\n", opt->desc);
      }
   }
}

static
char * indent(const char *str, int pad) {
   char *res = malloc(strlen(str) * 2);
   int   ind = -1;

   for (int i = 0; i < pad; ++i)
      res[++ind] =  ' ';

   do {
      res[++ind] = *str;
      if (*str == '\n')
         for (int i = 0; i < pad; ++i)
            res[++ind] = ' ';
   } while (*++str);

   return res;
}

static
char* fill_attrs(const char *s) {
   int state = 0;
   char *result = malloc(strlen(s) * 2);
   result[0] = 0;

   do {
      switch (*s) {
         case '*':
            strcat(result, attrs[0 + state]);
            state = !state;
            break;
         case '_':
            strcat(result, attrs[2 + state]);
            state = !state;
            break;
         case '\\':
            strncat(result, ++s, 1);
            break;
         default:
            strncat(result, s, 1);
      }
   } while (*++s);

   return result;
}

static void help_keys() {
   PA(
      "*Symbolic keys*\n"
      " Up/Down/Left/Right, PageUp/PageDown, Home/End,\n"
      " Insert/Delete, Space, Enter, Tab, Backspace, F1 .. F12\n"
      "\n"
      "*Modifiers*\n"
      " *Control*: Control-key, Ctrl-key, C-key, ^key\n"
      " *Alt*:     Alt-key, A-key, Meta-key, M-key\n"
      " *Shift*:   Shift-key, S-key\n"
   );
}

