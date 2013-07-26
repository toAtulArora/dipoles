#http://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html
#it is a useful link
#disable the graph plotting library if it is not installed
#disale the corresponding macro in main.c

USBFLAGS=   `libusb-config --cflags`
CVFLAGS=	`pkg-config --cflags opencv`
USBLIBS=    `libusb-config --libs`
CVLIBS=		`pkg-config --libs opencv`
EXTRAPARAM=	-std=c++11 -pthread -ggdb
GRAPHLIBS=	`pkg-config --libs plplotd-c++`
GRAPHFLAGS=	`pkg-config --cflags plplotd-c++`


EXE_SUFFIX=
EXTRAINCDIRS = include/

CC=				gcc
CCP=			g++
#this is cause I am trying to build a c code now!
#CFLAGS=			-O -g -Wall $(USBFLAGS)
CFLAGS=			 -O2 -g -Wall $(USBFLAGS)
CPFLAGS=		$(EXTRAPARAM) -g -Wall $(CVFLAGS) $(USBFLAGS)  $(GRAPHFLAGS)
LIBS=			$(USBLIBS) $(CVLIBS) $(GRAPHLIBS)

# OBJ=		main.o include/hiddata.o include/usbIO.o
OBJ=latticeAnalyser.o include/hiddata.o include/usbIO.o
PROGRAM=latticeAnalyser


all: $(PROGRAM)	

$(PROGRAM): $(OBJ)
	@echo
	@echo LINKING EVERYTHING TOGETHER NOW
	$(CCP) $(LIBS) -o $(PROGRAM) $(OBJ)  -I$(EXTRAINCDIRS)	


# strip: $(PROGRAM)
# 	strip $(PROGRAM)

clean:
	rm -f $(OBJ) $(PROGRAM)

.cpp.o:
	@echo
	@echo BUILDING CPP TARGETS TO OBJECTS
	$(CCP) $(CPFLAGS) -I$(EXTRAINCDIRS) -c $*.cpp -o $*.o
.c.o:
	@echo
	@echo BUILDING C TARGETS TO OBJECTS
	$(CC) $(CFLAGS) -I$(EXTRAINCDIRS) -c $*.c -o $*.o