
// solutin with poll:

      #define REDIRECT_BUFSZ 4096
      char buffer[REDIRECT_BUFSZ];
      ssize_t bytes_read;

      struct pollfd fds[2] = {
         // STDIN must be first, we use poll with size of 1 later
         { .fd = STDIN_FILENO,         .events = POLLIN },
         { .fd = context.fd_program,   .events = POLLIN }
      };

      TermKeyKey key;

      TermKeyKey escape;
      escape.type = TERMKEY_TYPE_KEYSYM;
      escape.code.sym = TERMKEY_SYM_ESCAPE;
      escape.modifiers = 0;

      for (;;) {
         /*
         poll(fds, 2, -1);

         if (fds[1].revents & POLLIN != 0) {
            if ((bytes_read = read(context.fd_program, &buffer, REDIRECT_BUFSZ)) != -1)
               write(STDOUT_FILENO, &buffer, bytes_read);
         }

         else if (fds[0].revents & POLLIN != 0) {
         */
            c = getchar();
            buf[bufi++] = c;

            if (c == 033) {
               if (poll(fds, 1, escdelay_ms) >= 0 && fds[0].revents & POLLIN != 0) {
                  goto PUSH_NON_ESCAPE;
               }
               else {
                  handle_key(&escape, buf, bufi);
                  bufi = 0;
                  continue;
               }
            }

            PUSH_NON_ESCAPE:
            termkey_push_bytes(tk, (char*) &c, 1);

            if (termkey_getkey(tk, &key) == TERMKEY_RES_KEY) {
               handle_key(&key, buf, bufi);
               bufi = 0;
            }
         //}



void log2(const char *fmt, ...) {
   FILE *fh = fopen("/tmp/log", "a");
   va_list ap;
   va_start(ap, fmt);
   vfprintf(fh, fmt, ap);
   va_end(ap);
   fclose(fh);
}


static char* tkType2Sym(int type) {
   switch (type) {
      case TERMKEY_TYPE_UNICODE:     return "UNICODE";
      case TERMKEY_TYPE_KEYSYM:      return "KEYSYM";
      case TERMKEY_TYPE_FUNCTION:    return "FUNCTION";
      case TERMKEY_TYPE_POSITION:    return "POSITION";
      case TERMKEY_TYPE_UNKNOWN_CSI: return "UNKNOWN_CSI";
      case TERMKEY_TYPE_OSC:         return "OSC";
      case TERMKEY_TYPE_DCS:         return "DCS";
      default:                       return "???";
   }
}

static int tofront(int c) {
   if (! buf[0] && !buf[1]) {
      buf[0] = c;
   }
   else {
      buf[1] = buf[0];
      buf[0] = c;
   }
}

static int toend(int c) {
   if (buf[0] && buf[1]) {
      buf[0] = buf[1];
      buf[1] = c;
   }
   else if (buf[0] && !buf[1]) {
      buf[1] = c;
   }
   else if (! buf[0]) {
      buf[0] = c;
   }
}

static int peek() {
   return buf[0];
}

static int pop() {
   int c = buf[0];
   buf[0] = buf[1];
   buf[1] = 0;
   return c;
}

   tofront(1);
   tofront(2);

   printf("%d\n", peek());
   printf("%d\n", pop());

   printf("%d\n", peek());
   printf("%d\n", pop());

   printf("%d\n", peek());
   printf("%d\n", pop());

   buf[0] = buf[1] = 0;
