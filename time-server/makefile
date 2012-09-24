CXX=gcc

CFLAGS=-g -c  -D_POSIX_PTHREAD_SEMANTICS

BASEDIR=.

INC=-I$(BASEDIR)/inc


OBJDIR=$(BASEDIR)/obj

BINDIR=$(BASEDIR)/bin

SRCDIR=$(BASEDIR)/src

LIBS= -lnsl -lpthread -lsocket


CLI_OBJS=$(OBJDIR)/CS533_Client.o \
	 $(OBJDIR)/utils.o

ECHO_OBJS=$(OBJDIR)/echocli.o \
	  $(OBJDIR)/utils.o

DAYTIME_OBJS=$(OBJDIR)/echotimecli.o \
	     $(OBJDIR)/utils.o

SERV_OBJS=$(OBJDIR)/services.o \
	  $(OBJDIR)/CS533_Server.o \
	  $(OBJDIR)/utils.o

all : server client echocli daytimecli

server : $(SERV_OBJS)
	$(CXX) -o $(BINDIR)/server $(SERV_OBJS) $(INC) $(LIBS)

client : $(CLI_OBJS)
	$(CXX) -o $(BINDIR)/client $(CLI_OBJS) $(INC) $(LIBS)

echocli : $(ECHO_OBJS)
	$(CXX) -o $(BINDIR)/echocli $(ECHO_OBJS) $(INC) $(LIBS)

daytimecli : $(DAYTIME_OBJS)
	$(CXX) -o $(BINDIR)/daytimecli $(DAYTIME_OBJS) $(INC) $(LIBS)


$(OBJDIR)/echotimecli.o : $(SRCDIR)/echotimecli.c
	$(CXX) $(CFLAGS) $(INC)  $(SRCDIR)/echotimecli.c -o $@

$(OBJDIR)/utils.o : $(SRCDIR)/utils.c
	$(CXX) $(CFLAGS) $(INC)  $(SRCDIR)/utils.c -o $@

$(OBJDIR)/echocli.o : $(SRCDIR)/echocli.c
	$(CXX) $(CFLAGS) $(INC)  $(SRCDIR)/echocli.c -o $@

$(OBJDIR)/CS533_Client.o : $(SRCDIR)/CS533_Client.c
	$(CXX) $(CFLAGS) $(INC)  $(SRCDIR)/CS533_Client.c -o $@

$(OBJDIR)/CS533_Server.o : $(SRCDIR)/CS533_Server.c
	$(CXX) $(CFLAGS) $(INC)  $(SRCDIR)/CS533_Server.c -o $@

$(OBJDIR)/services.o : $(SRCDIR)/services.c
	$(CXX) $(CFLAGS) $(INC)  $(SRCDIR)/services.c -o $@

clean : 
	rm $(OBJDIR)/*.o
	rm $(BINDIR)/*
