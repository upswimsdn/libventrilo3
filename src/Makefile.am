
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libventrilo3.la

libventrilo3_la_SOURCES = \
		api.c \
		data.c \
		dsp.c \
		encryption.c \
		handshake.c \
		libventrilo3.c \
		libventrilo3.h \
		message.c \
		message.h \
		network.c

libventrilo3_la_LDFLAGS = -version-info 1:0:0

libventrilo3_la_LIBADD = $(speex_LIBS) $(opus_LIBS) $(speexdsp_LIBS) -lm

.PHONY: dev

dev:
	rm -f libventrilo3.a
	gcc -pthread -Wall -Wextra -g -c -I. -I.. api.c          -o api_g.o
	gcc -pthread -Wall -Wextra -g -c -I. -I.. data.c         -o data_g.o
	gcc -pthread -Wall -Wextra -g -c -I. -I.. dsp.c          -o dsp_g.o
	gcc -pthread -Wall -Wextra -g -c -I. -I.. encryption.c   -o encryption_g.o
	gcc -pthread -Wall -Wextra -g -c -I. -I.. handshake.c    -o handshake_g.o
	gcc -pthread -Wall -Wextra -g -c -I. -I.. libventrilo3.c -o libventrilo3_g.o
	gcc -pthread -Wall -Wextra -g -c -I. -I.. message.c      -o message_g.o
	gcc -pthread -Wall -Wextra -g -c -I. -I.. network.c      -o network_g.o
	ar rvs libventrilo3.a api_g.o data_g.o dsp_g.o encryption_g.o handshake_g.o libventrilo3_g.o message_g.o network_g.o
	rm -f api_g.o data_g.o dsp_g.o encryption_g.o handshake_g.o libventrilo3_g.o message_g.o network_g.o

