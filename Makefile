PROGNAME   = termappwrapper
LIBS 		  = -lutil -ltermkey -lcurses -lpthread -lreadline
PREFIX     = /usr
DEBUG 	  = 0
FREE  	  = 0
STRIP      = strip
CC_FLAGS   = -Wall

ifeq ($(DEBUG), 1)
	FREE = 1
	CC_FLAGS += -g
	STRIP = true
else
	CC_FLAGS += -O3
endif

ifeq ($(README), 1)
CC_FLAGS += -DBOLD='"**"' -DBOLD_END='"**"' -DITALIC='"_"' -DITALIC_END='"_"'
endif

CC_FLAGS += -DFREE_MEMORY=$(FREE) -DDEBUG=$(DEBUG)

CMDS = goto ignore key load mask pass readline signal write
CMDS := $(addprefix cmd_, $(CMDS))

OBJS  = termkeystuff bind_parse iwrap conf lexer options commands help error_messages $(CMDS)
OBJS := $(addsuffix .o, $(OBJS))

build: $(OBJS)
	$(CC) $(CC_FLAGS) $(LIBS) objs/*.o main.c -o $(PROGNAME)
	$(STRIP) $(PROGNAME)

%.o:
	@mkdir -p objs
	$(CC) $(CC_FLAGS) $(LIBS) -c $*.c -o objs/$*.o

install:
	install -m 0755 $(PROGNAME) $(PREFIX)/bin/$(PROGNAME)

README.md: .force
	./termappwrapper -h all > README.md

doc: .force
	./tools/genhelp.py > $(PROGNAME).md
	go-md2man -in $(PROGNAME).md -out $(PROGNAME).1

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

