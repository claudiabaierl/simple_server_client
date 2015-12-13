##
## @file simple_message_server.c
## @file simple_message_client.c
## Verteilte Systeme TCP/IP Implementierung Client und Server
## 
## @author Claudia Baierl  ic14b003 <ic14b003@technikum-wien.at>
## @author Zübide Sayici ic14b002 <ic14b002@technikum-wien.at>
## @date 2015/11/26
## 
## @version $Revision: 10 $
## 
## 
## 
##

##
## ------------------------------------------------------------- variables --
##


CC=gcc52
CFLAGS=-Wall -Werror -Wextra -Wstrict-prototypes -pedantic -fno-common -g -O3 -std=gnu11
CFLGS2=-Wall -Werror -Wextra -Wstrict-prototypes -pedantic -fno-common -g -O3 -o simple_message_client simple_message_client.o -lsimple_message_client_commandline_handling
CFLGS3=-Wall -Werror -Wextra -Wstrict-prototypes -pedantic -fno-common -g -O3 -o simple_message_server simple_message_server.o
GREP=grep
DOXYGEN=doxygen


OBJECTS= simple_message_client.o simple_message_server.o

EXCLUDE_PATTERN=footrulewidth

##
## ----------------------------------------------------------------- rules --
##

## jedes Object-File haengt vom gleichnamigen C-File ab
%.o : %.c
	## gcc kompiliert .c zu .o
	$(CC) $(CFLAGS) -c $<

##
## --------------------------------------------------------------- targets --
##

## "make all"
all: client_server


## client_server haengt von allen Eintraegen in der Liste OBJECTS ab
client_server: $(OBJECTS)
	$(CC) $(CFLGS2) && \
$(CC) $(CFLGS3) 

clean:
	rm -f *.o simple_message_client simple_message_server ok.png vcs_tcpip_bulletin_board_response.html
  

distclean: clean
	rm -f -r doc

## doc haengt von pdf ab, (welches von html abhängt)
doc: pdf

## html ruft doxygen auf
html:
	$(DOXYGEN) doxygen.dcf


## pdf haengt von html ab, (welches doxygen aufruft)
pdf: html
	cd doc/pdf && \
	mv refman.tex refman_save.tex && \
	$(GREP) -v $(EXCLUDE_PATTERN) refman_save.tex > refman.tex && \
	rm -f refman_save.tex && \
	make && \
	mv refman.pdf refman.save && \
	rm -f *.pdf *.html *.tex *.aux *.sty *.log *.eps *.out *.ind *.idx \
	      *.ilg *.toc *.tps *.md5 *.ttf Makefile && \
	mv refman.save refman.pdf

##
## ---------------------------------------------------------- dependencies --
##

##
## =================================================================== eof ==
##
