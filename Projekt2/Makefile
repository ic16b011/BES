##
## @file Makefile
## Betriebssysteme mypopen Makefile
## Beispiel 2
##
## @author David Sattler <ic16b036@technikum-wien.at> 
## @date 2017/05/01
##
## @version 1
##
## @todo
##
## Last Modified: $Author: David $
##

##
## ------------------------------------------------------------- variables --
##

CC=gcc52
CFLAGS=-DDEBUG -Wall -pedantic -Werror -Wextra -Wstrict-prototypes -fno-common -g -c -O3 -std=gnu11
CP=cp
CD=cd
MV=mv
GREP=grep
DOXYGEN=doxygen

OBJECTS=mypopen.o

EXCLUDE_PATTERN=footrulewidth

##
## ----------------------------------------------------------------- rules --
##

%.o: %.c
	$(CC) $(CFLAGS)  $<


##
## --------------------------------------------------------------- targets --
##

.PHONY: all
all: mypopen

popentest: mypopen.c
	gcc52 -o popentest mypopen.c -lpopentest

test-pipe: mypopen.c
	gcc52 -o test-pipe mypopen.c -ltest-pipe

mypopen: $(OBJECTS)
	$(CC) $(CFLAGS) -o mypopen.o mypopen.c

.PHONY: clean
clean:
	$(RM) *.o *~ mypopen popentest test-pipe

.PHONY: distclean
distclean: clean
	$(RM) -r doc

doc: html pdf

.PHONY: html
html:
	$(DOXYGEN) doxygen.dcf

pdf: html
	$(CD) doc/pdf && \
	$(MV) refman.tex refman_save.tex && \
	$(GREP) -v $(EXCLUDE_PATTERN) refman_save.tex > refman.tex && \
	$(RM) refman_save.tex && \
	make && \
	$(MV) refman.pdf refman.save && \
	$(RM) *.pdf *.html *.tex *.aux *.sty *.log *.eps *.out *.ind *.idx \
	      *.ilg *.toc *.tps Makefile && \
	$(MV) refman.save refman.pdf

##
## ---------------------------------------------------------- dependencies --
##

##
## =================================================================== eof ==
##
