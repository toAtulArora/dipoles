USBFLAGS=   `libusb-config --cflags`
USBLIBS=    `libusb-config --libs`
EXE_SUFFIX=
EXTRAINCDIRS = include/

CC=				gcc
CFLAGS=			-O -g -Wall $(USBFLAGS)
LIBS=			$(USBLIBS)

OBJ=		main.o include/hiddata.o include/usbIO.o
PROGRAM=	main

all: $(PROGRAM)

$(PROGRAM): $(OBJ)
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS) -I$(EXTRAINCDIRS)

strip: $(PROGRAM)
	strip $(PROGRAM)

clean:
	rm -f $(OBJ) $(PROGRAM)

.c.o:
	$(CC) $(ARCH_COMPILE) $(CFLAGS) -I$(EXTRAINCDIRS) -c $*.c -o $*.o
