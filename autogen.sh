#! /bin/sh
mkdir -p build-aux
glib-gettextize 
libtoolize
autoreconf -i -v
