# Define required macros here

BASEDIR = .
SRCDIR = $(BASEDIR)/src
INCDIR = $(BASEDIR)/inc
LIBDIR = $(BASEDIR)/lib
OBJDIR = $(BASEDIR)/obj
BINDIR = $(BASEDIR)/bin

CXX=gcc
CXXFLAGS = -g -Wall 

ODR_OBJECTS = $(OBJDIR)/get_hw_addrs.o \
	      $(OBJDIR)/ODR.o \
	      $(OBJDIR)/tableDDT.o \
	      $(OBJDIR)/packetQ.o \
	      $(OBJDIR)/handlers.o \
	      $(OBJDIR)/odr_client.o \
	      $(OBJDIR)/odr_clientsrv.o

TIME_SRV_OBJECTS = $(OBJDIR)/odr_client.o \
	           $(OBJDIR)/odr_clientsrv.o \
		   $(OBJDIR)/time_srv.o

TIME_CLI_OBJECTS = $(OBJDIR)/odr_client.o \
	           $(OBJDIR)/odr_clientsrv.o \
		   $(OBJDIR)/get_hw_addrs.o \
		   $(OBJDIR)/time_cli.o \
		   $(OBJDIR)/tableDDT.o
INCLUDES =  -I$(INCDIR) 
LIBS = -lresolv -lnsl 

all:  $(BINDIR)/ODR $(BINDIR)/time_srv $(BINDIR)/time_cli

$(BINDIR)/ODR: $(ODR_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)/ODR $(INCLUDES) $(ODR_OBJECTS) $(LIBS)

$(BINDIR)/time_srv: $(TIME_SRV_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)/time_srv $(INCLUDES) $(TIME_SRV_OBJECTS) $(LIBS)

$(BINDIR)/time_cli: $(TIME_CLI_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)/time_cli $(INCLUDES) $(TIME_CLI_OBJECTS) $(LIBS)

$(OBJDIR)/time_srv.o: $(SRCDIR)/time_srv.c
		      $(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/time_srv.o $(SRCDIR)/time_srv.c

$(OBJDIR)/time_cli.o: $(SRCDIR)/time_cli.c
		      $(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/time_cli.o $(SRCDIR)/time_cli.c

$(OBJDIR)/get_hw_addrs.o: $(SRCDIR)/get_hw_addrs.c
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/get_hw_addrs.o $(SRCDIR)/get_hw_addrs.c

$(OBJDIR)/ODR.o: $(SRCDIR)/ODR.c
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/ODR.o $(SRCDIR)/ODR.c

$(OBJDIR)/tableDDT.o: $(SRCDIR)/tableDDT.c
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/tableDDT.o $(SRCDIR)/tableDDT.c

$(OBJDIR)/handlers.o: $(SRCDIR)/handlers.c
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/handlers.o $(SRCDIR)/handlers.c

$(OBJDIR)/packetQ.o: $(SRCDIR)/packetQ.c
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/packetQ.o $(SRCDIR)/packetQ.c

$(OBJDIR)/odr_clientsrv.o: $(SRCDIR)/odr_clientsrv.c
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/odr_clientsrv.o $(SRCDIR)/odr_clientsrv.c

$(OBJDIR)/odr_client.o: $(SRCDIR)/odr_client.c
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/odr_client.o $(SRCDIR)/odr_client.c

clean:
	rm obj/*.o bin/*
