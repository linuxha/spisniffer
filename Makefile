CC	=	gcc
CFLAGS	=	 -g3 -Wall -Os -DTRUE=1 -DFALSE=0

VERSION	=	\"V0.15\"
CFLAGS	+=	-DVERSION=$(VERSION)
#LDFLAGS += 	-lcurses

#######################################################################

SRC	=	serial.c buspirate.c main.c cleanup.c
OBJ	=	serial.o buspirate.o main.o cleanup.o

all:	spisniffer

spisniffer:	$(OBJ)
	$(CC) -g3 -o spisniffer $(OBJ) $(LDFLAGS)

serial.o:    serial.c serial.h
buspirate.o: buspirate.c buspirate.h
main.o:      main.c
cleanup.o:   cleanup.c

clean:
	rm -f $(OBJ) spisniffer *~ foo core
