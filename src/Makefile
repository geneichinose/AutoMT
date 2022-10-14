### MacOSX
CC = clang
CFLAGS = -I/Users/ichinose1/Work/mxml-2.10 -I../include
XML_LIBS = -L/Users/ichinose1/Work/mxml-2.10 -lmxml

#### Linux
#CC = gcc -pthread
#CFLAGS = -I/home/ichinose1/Work/mxml-2.10 -I../include 
#XML_LIBS = -L/home/ichinose1/Work/mxml-2.10 -lmxml

#### ok down below
GETPAR_LIBS = -L../lib -lget
LIBS = -lm
INSTALL = $(HOME)/Work/AutoMT/bin

all : automt getdata read_stachan_xml read_events_xml

read_stachan_xml.o : read_stachan_xml.c
read_events_xml.o : read_events_xml.c

xmlsubs.o : xmlsubs.c ../include/event.h ../include/mytime.h
timesubs.o : timesubs.c ../include/mytime.h
iosubs.o : iosubs.c ../include/event.h ../include/mytime.h
automt.o : automt.c ../include/event.h ../include/mytime.h
mkdirp2.o : mkdirp2.c 
sacio.o : sacio.c ../include/sac.h  ../include/mytime.h ../include/sacfile.h
writelog.o : writelog.c
shorten_path.o : shorten_path.c

read_stachan_xml : read_stachan_xml.o xmlsubs.o timesubs.o writelog.o
	$(CC) $(CFLAGS) -o $@ read_stachan_xml.o xmlsubs.o timesubs.o writelog.o $(XML_LIBS) $(LIBS)
	mv $@ $(INSTALL)

read_events_xml : read_events_xml.o xmlsubs.o writelog.o
	$(CC) $(CFLAGS) -o $@ read_events_xml.o xmlsubs.o timesubs.o writelog.o $(XML_LIBS) $(LIBS)
	mv $@ $(INSTALL)

OBJS = automt.o xmlsubs.o timesubs.o iosubs.o mkdirp2.o sacio.o writelog.o shorten_path.o

automt : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(XML_LIBS) $(GETPAR_LIBS) $(LIBS)
	mv $@ $(INSTALL)

loadml : loadml.o
	$(CC) $(CFLAGS) -o $@ loadml.o timesubs.o -lm 
	mv $@ $(INSTALL)

OBJS_GETDATA = xmlsubs.o timesubs.o iosubs.o mkdirp2.o sacio.o shorten_path.o writelog.o

getdata : getdata.o $(OBJS_GETDATA)
	$(CC) $(CFLAGS) -o $@ getdata.o $(OBJS_GETDATA) $(XML_LIBS) $(GETPAR_LIBS) $(LIBS)
	mv $@ $(INSTALL)

xml2sacpaz : xml2sacpaz.o xmlsubs.o timesubs.o iosubs.o writelog.o
	$(CC) $(CFLAGS) -o $@ xml2sacpaz.o xmlsubs.o timesubs.o iosubs.o writelog.o  $(XML_LIBS) $(GETPAR_LIBS) $(LIBS)
	mv $@ $(INSTALL)

clean : 
	/bin/rm -f *.o 
