SUBDIRS = doc 

lib_LTLIBRARIES = libtableware.la

# Note that the include files from moz-sdk were taken from the 
# mozilla/dist/sdk/include directory of the Firefox 2.0.0.2 release.

# need better solution than hardcoded X11 include path, but
# 'pkg-config --cflags' does not work correctly for x11 or xproto packages
INCLUDES = \
	-I$(top_srcdir)/moz-sdk \
	`pkg-config --cflags dbus-1 xcb` \
	`pkg-config --cflags nspr` \
	-I/usr/include/X11

libtableware_la_LDFLAGS = \
	`pkg-config --libs` \
	`pkg-config --libs nspr`

libtableware_la_SOURCES = \
	tableware.c \
	npshell.c \
	npunix.c \
	tableware.h \
	moz-sdk/jni.h \
	moz-sdk/jni_md.h \
	moz-sdk/jri.h \
	moz-sdk/jri_md.h \
	moz-sdk/jritypes.h \
	moz-sdk/npapi.h \
	moz-sdk/npruntime.h \
	moz-sdk/nptypes.h \
	moz-sdk/npupp.h



# custom install rule
install-plugin:
	mkdir -p ~/.mozilla/plugins
	cp .libs/libtableware.so ~/.mozilla/plugins
