
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
