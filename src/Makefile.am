
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libventrilo3.la

libventrilo3_la_SOURCES = libventrilo3.c  account.c channel.c codec.c dsp.c \
	handshake.c message.c network.c rank.c user.c

libventrilo3_la_LDFLAGS = -version-info 1:0:0

libventrilo3_la_LIBADD = $(speex_LIBS) $(gsm_LIBS) -lm

