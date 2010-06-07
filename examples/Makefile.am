
ACLOCAL_AMFLAGS = -I m4

connection_SOURCES = connection.c

connection_LDADD = @top_srcdir@/src/libventrilo3.la

INCLUDES = -I@top_srcdir@/src/

noinst_PROGRAMS = connection

dev:
	gcc -pthread -Wall -Wextra -g -I../src -o connection connection.c ../src/.libs/libventrilo3.a

