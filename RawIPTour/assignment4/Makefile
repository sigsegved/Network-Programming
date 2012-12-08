# Define required macros here

BASEDIR = .
SRCDIR = $(BASEDIR)/src
INCDIR = $(BASEDIR)/inc
LIBDIR = $(BASEDIR)/lib
OBJDIR = $(BASEDIR)/obj
BINDIR = $(BASEDIR)/bin

CXX=gcc
CXXFLAGS = -g -Wall 

TOUR_OBJECTS =$(OBJDIR)/get_hw_addrs.o \
	      $(OBJDIR)/tour.o \
	      $(OBJDIR)/ARPI.o \
	      $(OBJDIR)/tableDDT.o 

ARP_OBJECTS = $(OBJDIR)/get_hw_addrs.o \
	      $(OBJDIR)/ARP.o \
	      $(OBJDIR)/tableDDT.o 
	     

ARPT_OBJECTS =$(OBJDIR)/get_hw_addrs.o \
	      $(OBJDIR)/ARP_TEST.o \
	      $(OBJDIR)/tableDDT.o 

AREQ_OBJECTS = $(OBJDIR)/ARPI.o

INCLUDES =  -I$(INCDIR) 
LIBS = -lresolv -lnsl 

all:  $(BINDIR)/arp_kuthaman $(BINDIR)/tour_kuthaman

$(BINDIR)/arp_kuthaman: $(AREQ_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)/arp_kuthaman $(INCLUDES) $(AREQ_OBJECTS) $(LIBS)
$(BINDIR)/tour_kuthaman: $(TOUR_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)/tour_kuthaman $(INCLUDES) $(TOUR_OBJECTS) $(LIBS)
$(BINDIR)/arp_kuthaman: $(ARP_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)/arp_kuthaman $(INCLUDES) $(ARP_OBJECTS) $(LIBS)

$(BINDIR)/ARP_TEST: $(ARPT_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)/ARP_TEST $(INCLUDES) $(ARPT_OBJECTS) $(LIBS)

$(OBJDIR)/get_hw_addrs.o: $(SRCDIR)/get_hw_addrs.c
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/get_hw_addrs.o $(SRCDIR)/get_hw_addrs.c

$(OBJDIR)/ARP.o: $(SRCDIR)/ARP.c
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/ARP.o $(SRCDIR)/ARP.c

$(OBJDIR)/ARPI.o: $(SRCDIR)/ARPI.c
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/ARPI.o $(SRCDIR)/ARPI.c

$(OBJDIR)/ARP_TEST.o: $(SRCDIR)/ARP_TEST.c
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/ARP_TEST.o $(SRCDIR)/ARP_TEST.c
$(OBJDIR)/tour.o: $(SRCDIR)/tour.c
		 $(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/tour.o $(SRCDIR)/tour.c
$(OBJDIR)/tableDDT.o: $(SRCDIR)/tableDDT.c
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $(OBJDIR)/tableDDT.o $(SRCDIR)/tableDDT.c
clean:
	rm obj/*.o bin/* server.sock.uk 

