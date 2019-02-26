#include "conf.h"
#include "iwrap.h"

#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define CFG_DIR_NAME "termappwrapper"
#define SYSTEM_DIR ""

static
int load_conf_at(const char *dir, const char *f) {
   int  ret = 0;
   char oldcwd[4096];

   if (! getcwd(oldcwd, sizeof(oldcwd)))
      return 0;

   if (chdir(dir))
      return 0;

   if (! access(f, F_OK))
      ret = read_conf_file(f);
   else
      write_error("%s: %s", f, strerror(ENOENT));

   if (chdir(oldcwd))
      errx(1, "Could not go to old working directory");

   return ret;
}

int load_conf(const char *f) {
   char dir[4096];

   if (strchr(f, '/'))
      return read_conf_file(f);

   if (load_conf_at(".", f))
      return 1;

   const char *xdg_home = getenv("XDG_CONFIG_HOME");
   if (xdg_home) {
      sprintf(dir, "%s/%s", xdg_home, CFG_DIR_NAME);
      if (load_conf_at(dir, f))
         return 1;
   }

   const char *home = getenv("HOME");
   if (home) {
      sprintf(dir, "%s/.config/%s", home, CFG_DIR_NAME);
      if (load_conf_at(dir, f))
         return 1;

      sprintf(dir, "%s/.%s", home, CFG_DIR_NAME);
      if (load_conf_at(dir, f))
         return 1;
   }

   return 0;
}

static COMMAND_CALL_FUNC(call) {
   load_conf((char*) cmd->arg);
}

static COMMAND_PARSE_FUNC(parse) {
   return (void*) strdup(args[0]);
}

const command_t command_load = {
   .name  = "load",
   .desc  = "Load configuration file\n"
            "If file is a sole filename it will be\n"
            "searched in the following places:\n"
            " - $PWD\n"
            " - $XDG\\_CONFIG\\_HOME/.termappwrapper/\n"
            " - $HOME/.config/termappwrapper/\n"
            " - $HOME/.termappwrapper/",
   .args  = (const char*[]) { "FILE", 0 },
   .opts  = NULL,
   .call  = &call,
   .parse = &parse,
   .free  = &free
};
