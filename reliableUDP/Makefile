CC = gcc

LIBS = -lresolv -lsocket -lnsl -lpthread\
	/home/courses/cse533/Stevens/unpv13e_solaris2.10/libunp.a\
	
FLAGS = -g -O2

CFLAGS = ${FLAGS} -I/home/courses/cse533/Stevens/unpv13e_solaris2.10/lib

all: client server

letsdothis.o : letsdothis.c
	${CC} ${CFLAGS} -c letsdothis.c
recv.o : recv.c
	${CC} ${CFLAGS} -c recv.c

get_ifi_info_plus.o: get_ifi_info_plus.c
	${CC} ${CFLAGS} -c get_ifi_info_plus.c

client: udpcli.o get_ifi_info_plus.o recv.o
	${CC} ${FLAGS} -o client udpcli.o get_ifi_info_plus.o recv.o ${LIBS}	

udpcli.o: udpcli.c
	${CC} ${CFLAGS} -c udpcli.c
	
server: server.o get_ifi_info_plus.o letsdothis.o
	${CC} ${FLAGS} -o server server.o get_ifi_info_plus.o letsdothis.o ${LIBS}

server.o: server.c 
	${CC} ${CFLAGS} -c server.c 

clean:
	rm *.o client server

	

