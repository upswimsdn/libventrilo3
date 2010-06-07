
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libventrilo3.la

libventrilo3_la_SOURCES = libventrilo3.c libventrilo3.h

libventrilo3_la_LDFLAGS = -version-info 1:0:0

libventrilo3_la_LIBADD = $(gsm_LIBS) $(speex_LIBS) $(celt_LIBS) -lm

dev:
	gcc -pthread -Wall -Wextra -g -c -I. -I.. -o libventrilo3.o libventrilo3.c

