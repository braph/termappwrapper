LEX = /usr/bin/flex
CFLAGS = -g
LDLIBS = -lfl
CC = /usr/bin/g++
YACC = /usr/bin/yacc

build: #cfgparser.scan.c 
	g++ -O2 wrapper.cpp -lutil -lpthread -lncurses -o wrapper #-lfl
#cfgparser.scan.cpp cfgparser.tab.c 

debug:
	g++ -g  wrapper.cpp -lutil -lpthread -lncurses -o wrapper

cfgparser.scan.c:  cfgparser.tab.c
	flex -+ cfgparser.l
	mv -f lex.yy.cc cfgparser.scan.cpp

cfgparser.tab.c:
	yacc -d cfgparser.y
	mv -f y.tab.c cfgparser.tab.c
	mv -f y.tab.h cfgparser.tab.h

cfgparser.tab.h:
	yacc -d cfgparser.y
	mv -f y.tab.c cfgparser.tab.c
	mv -f y.tab.h cfgparser.tab.h

clean:
	rm -f cfgparser.scan.cpp cfgparser.tab.h cfgparser.tab.c
