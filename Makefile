CC = gcc
#CC = ncc -ncgcc -ncld -ncfabs
#AR = nccar
#LD = nccld
RM=rm -f
DEPEND = makedepend
CDEBUGFLAGS = -Wall -pedantic -g
EXTRA_DEFINES = # -DSUNOS
CFLAGS = $(CDEBUGFLAGS) -O3 $(EXTRA_DEFINES) #-DDEBUG
EXTRA_LIBRARIES = #-lnsl -lsocket

SRCS = action.c bot.c compare.c list.c mode.c signal.c time.c aion.c cell.c ctcp.c match.c parse.c socket.c strip.c file.c encode.c debug.c client.c

OBJS = action.o bot.o compare.o list.o mode.o signal.o time.o aion.o cell.o ctcp.o match.o parse.o socket.o strip.o file.o encode.o debug.o client.o

 PROGRAM = aion

all:: aion

aion: $(OBJS)
	$(RM) $@
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(EXTRA_LIBRARIES)

depend::
	$(DEPEND) -- $(EXTRA_DEFINES) -- $(SRCS)

clean::
	$(RM) aion *.CKP *.ln *.BAK *.bak *.o core errs ,* *~ *.a .emacs_* tags TAGS make.log MakeOut  "#"* *.html bot*.log


action.o: types.h time.h action.h
action.o: defs.h socket.h strip.h
bot.o: time.h defs.h types.h socket.h list.h bot.h
compare.o: defs.h types.h time.h match.h compare.h
list.o: types.h time.h defs.h match.h cell.h list.h
mode.o: time.h defs.h types.h mode.h
mode.o: compare.h list.h cell.h action.h match.h strip.h
signal.o: signal.h 
time.o: time.h defs.h
aion.o: time.h defs.h types.h bot.h
aion.o: list.h cell.h socket.h parse.h compare.h file.h signal.h
cell.o: time.h defs.h types.h list.h compare.h socket.h cell.h
ctcp.o: time.h defs.h ctcp.h
ctcp.o: types.h socket.h action.h version.h
parse.c: types.h client.h defs.h bot.h
client.o: defs.h types.h time.h list.h action.h encode.h debug.h parse.c parse.h
#client.o: cell.h compare.h bot.h mode.h socket.h ctcp.h strip.h match.h file.h
#	$(CC) $(CDEBUGFLAGS) -O0  -c client.c -o client.o
socket.o: signal.h socket.h
strip.o: defs.h match.h strip.h
file.o: defs.h types.h time.h list.h cell.h compare.h file.h
debug.o: defs.h types.h time.h list.h debug.h
match.o: match.h
