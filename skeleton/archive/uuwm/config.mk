# uuwm version
VERSION = 0.1

# Customize below to fit your system

# paths
PREFIX = /usr/local

# libs
L=xcb xcb-aux xcb-atom xcb-icccm glib-2.0

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\"
CFLAGS = -std=c99 -pedantic -Wall -O0 -g $(CPPFLAGS) $(foreach lib,$(L),$(shell pkg-config --cflags $(lib)))
LDFLAGS = $(foreach lib,$(L),$(shell pkg-config --libs $(lib)))
