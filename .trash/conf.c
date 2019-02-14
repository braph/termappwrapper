
static int c_goto(int argc, char *args[]) {
   if (! check_args(argc, "mode", 0))
      return 0;

   if (! context.current_mode = get_keymode(args[0])) {
      write_error("unknown mode: %s", args[0]);
      return 0;
   }

   return 1;
}

