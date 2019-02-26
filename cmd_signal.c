#include "iwrap.h"

#include <stdint.h>
#include <string.h>
#include <signal.h>

static struct {
   uint8_t     number;
   char        *name;
} signals[] = {
   { SIGINT,  "INT"  },
   { SIGHUP,  "HUP"  },
   { SIGTERM, "TERM" },
   { SIGKILL, "KILL" },
   { SIGTSTP, "TSTP" },
   { SIGSTOP, "STOP" },
   { SIGCONT, "CONT" },
   { SIGUSR1, "USR1" },
   { SIGUSR2, "USR2" }
};
#define SIGNAL_SIZE (sizeof(signals)/sizeof(signals[0]))

static
int name2signal(const char *name) {
   if (! strncasecmp(name, "SIG", 3))
      name += 3;

   for (int i = SIGNAL_SIZE; i--; )
      if (! strcasecmp(name, signals[i].name))
         return signals[i].number;

   return 0;
}

/*
static
char* signal2name(int number) {
   for (int i = 0; i < SIGNAL_SIZE; ++i)
      if (signals[i].number == number)
         return signals[i].name;

   return NULL;
}
*/

static COMMAND_CALL_FUNC(call) {
   int sig = (int) (uintptr_t) cmd->arg;
   kill(context.program_pid, sig);
}

static COMMAND_PARSE_FUNC(parse) {
   int number;

   if (! (number = name2signal(args[0])))
      write_error("unknown signal: %s", args[0]);

   return (void*) (uintptr_t) number; // is NULL if failed
}

const command_t command_signal = {
   .name  = "signal",
   .desc  = "Send signal to program",
   .args  = (const char *[]) { "SIGNAL", 0 },
   .opts  = NULL,
   .parse = &parse,
   .call  = &call,
   .free  = NULL
};
