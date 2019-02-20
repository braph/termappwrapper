
command_call_t*
command_call_new(command_t *command, void *arg)
{
   command_call_t *call = malloc(sizeof(*call));
   call->arg            = arg;
   call->command        = command;
   return call;
}

int check_n_args(int argc, int n) {
   if (n == '+') {
      if (argc < 1) {
         write_error("missing argument");
         return 0;
      }

      return 1;
   }

   if (argc < n) {
      write_error("missing argument%s", (n > 1 ? "s" : ""));
      return 0;
   }

   if (argc > n) {
      write_error("spare arguments");
      return 0;
   }

   return 1;
}

char *args_find_optind(int argc, char *args[]) {
   int pos = 0;

   for (int i = 0; i < argc; ++i) {
      if (argv[i][0] == '-') {
         if (argv[i][1] == '-') {
            return i + 1;
         }
         else if (pos) {
            write_error("found option after argument list: %s\n", argv[i]);
            return -1;
         }
         else {
            pos = i + 1;
         }
      }
   }

   return pos;
}

