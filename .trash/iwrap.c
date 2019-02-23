
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

void* cmd_parse_none(int argc, char **args) {
   if (! check_args(argc, 0))
      return NULL;
   return (void*)1;
}

   /*
   char *s = get_key_code(key);
   if   (s)  write(context.program_fd, s, strlen(s));
   fprintf(stderr, "%10s: ", format_key(key));
   for (int i = 0; i < len; ++i) fprintf(stderr, " %3d", raw[i]);
   for (int i = len; i < 9; ++i) fprintf(stderr, " %3d", 0);
   fprintf(stderr, "\n");
   */

   /*
   printf("ign=%d, type=%d resul=%d\n",
         context.current_mode->ignore_unmapped,
         TERMKEY_TYPE_TO_FLAG(key->type),
         context.current_mode->ignore_unmapped & TERMKEY_TYPE_TO_FLAG(key->type));
   */

   /*
   if (key->type == TERMKEY_TYPE_UNICODE) {
      if (key->code.codepoint == 's') {
   */

