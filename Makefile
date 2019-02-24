PROGNAME   = termappwrapper
LIBS 		  = -lutil -ltermkey -lcurses -lpthread -lreadline
PREFIX     = /usr
DEBUG 	  = 0
FREE  	  = 0
CC_FLAGS   = -Wall -O2

ifeq ($(DEBUG), 1)
	FREE = 1
	CC_FLAGS += -g
endif

CC_FLAGS += -DFREE_MEMORY=$(FREE) -DDEBUG=$(DEBUG)

CMDS = goto mask write key signal ignore readline
CMDS := $(addprefix cmd_, $(CMDS))

OBJS  = termkeystuff bind_parse iwrap conf lexer common options commands $(CMDS)
OBJS := $(addsuffix .o, $(OBJS))

build: $(OBJS)
	$(CC) $(CC_FLAGS) $(LIBS) objs/*.o main.c -o $(PROGNAME)
	strip $(PROGNAME)

%.o:
	@mkdir -p objs
	$(CC) $(CC_FLAGS) $(LIBS) -c $*.c -o objs/$*.o

install:
	install -m 0755 $(PROGNAME) $(PREFIX)/bin/$(PROGNAME)

vi_conf.h: .force
	./tools/stripconf.py -c -v VI_CONF confs/vi.conf > vi_conf.h

game_conf.h: .force
	./tools/stripconf.py -c -v GAME_CONF confs/game.conf > game_conf.h

clean:
	rm -f $(PROGNAME)
	rm -rf objs

nanotest:
	./termappwrapper -v nano vi_conf.h

.force:
	@true

