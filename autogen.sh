#! /bin/sh
mkdir -p acaux
glib-gettextize 
libtoolize
autoreconf -i -v
