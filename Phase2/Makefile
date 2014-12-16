ALL: default 

CC           = gcc
CLINKER      = $(CC)
OPTFLAGS     = -O0


SHELL = /bin/sh

CFLAGS  =   -DREENTRANT -Wunused -Wall -g
CCFLAGS = $(CFLAGS)
LIBS =  -lpthread -lm

EXECS = exemple common.o dsmexec dsmwrap truc

default: $(EXECS)

dsmexec: dsmexec.o common.o
	$(CLINKER) $(OPTFLAGS) -o dsmexec dsmexec.o  common.o $(LIBS)
	
dsmwrap: dsmwrap.o common.o
	$(CLINKER) $(OPTFLAGS) -o dsmwrap dsmwrap.o  common.o $(LIBS)

exemple: exemple.o dsm.o common.o
	$(CLINKER) $(OPTFLAGS) -o exemple exemple.o dsm.o common.o $(LIBS)

truc: truc.o
	$(CLINKER) $(OPTFLAGS) -o truc truc.o common.o $(LIBS)	

clean:
	@-/bin/rm -f *.o *~ PI* $(EXECS) *.out core  
.c:
	$(CC) $(CFLAGS) -o $* $< $(LIBS)
.c.o:
	$(CC) $(CFLAGS) -c $<
.o:
	${CLINKER} $(OPTFLAGS) -o $* $*.o $(LIBS)
