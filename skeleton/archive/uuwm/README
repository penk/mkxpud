uuwm - micro-micro window manager
============================
uuwm is an extremely fast and small window manager for embedded devices.

uuwm is based on dwm (see README.dwm for details).

Requirements
------------
In order to build uuwm you need the Xlib and XRandR header files and libraries.


Installation
------------
Edit config.mk to match your local setup (uuwm is installed into
the /usr/local namespace by default).

Afterwards enter the following command to build and install uuwm (if
necessary as root):

    make clean install

If you are going to use the default bluegray color scheme it is highly
recommended to also install the bluegray files shipped in the dextra package.


Running uuwm
-----------
Add the following line to your .xinitrc to start uuwm using startx:

    exec uuwm

In order to connect uuwm to a specific display, make sure that
the DISPLAY environment variable is set correctly, e.g.:

    DISPLAY=foo.bar:1 exec uuwm

(This will start uuwm on display :1 of the host foo.bar.)

Configuration
-------------
uuwm does not have any configuration.
