
ACLOCAL_AMFLAGS = -I m4

connection_SOURCES = connection.c

connection_LDADD = @top_srcdir@/src/libventrilo3.la

INCLUDES = -I@top_srcdir@/src/

noinst_PROGRAMS = connection

.PHONY: dev jukebox

dev:
	cd ../src/ && make dev
	gcc -pthread -Wall -Wextra -g -I.. -I../src/ -o connection \
		connection.c \
		../src/libventrilo3.o \
		$(speex_LIBS) \
		$(celt_LIBS) \
		$(speexdsp_LIBS) \
		$(LIBS) \
		-lm

jukebox:
	cd ../src/ && make dev
	gcc -pthread -Wall -g -I.. -I../src/ -o jukebox \
		`if pkg-config flac; then echo -n "-DHAVE_FLAC"; fi` \
		jukebox.c \
		../src/libventrilo3.o \
		$(speex_LIBS) \
		$(celt_LIBS) \
		$(speexdsp_LIBS) \
		-lmpg123 \
		`if pkg-config flac; then echo -n "-lFLAC"; fi` \
		$(LIBS) \
		-lm

