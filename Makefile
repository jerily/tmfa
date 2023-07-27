ifndef NAVISERVER
    NAVISERVER  = /usr/local/ns
endif

#
# Module name
#
MOD      =  libtmfa.so

#
# Objects to build.
#
MODOBJS     = library.o

MODLIBS  += -lcotp

include  $(NAVISERVER)/include/Makefile.module