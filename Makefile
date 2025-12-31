# C0_INSTALL is the directory containing the c0 deployment
# e.g., /usr/local/share/cc0 if cc0 is at /usr/local/share/cc0/bin/cc0
C0_TMP=$(shell dirname `which cc0`)
C0_INSTALL=$(shell dirname $(C0_TMP))
C0LIBDIR=$(C0_INSTALL)/lib
C0RUNTIMEDIR=$(C0_INSTALL)/runtime

# Compiling c0vm
CFLAGS=-fwrapv -Wall -Wextra -Werror -g
CC=gcc -std=c99 -pedantic
CC_FAST:=$(CC) $(CFLAGS)
CC_SAFE:=$(CC) $(CFLAGS) -fsanitize=undefined -DDEBUG

LINKERFLAGS=-L$(C0LIBDIR) -L$(C0RUNTIMEDIR) -Wl,-rpath $(C0LIBDIR) -Wl,-rpath $(C0RUNTIMEDIR) -limg -lstring -lcurses -largs -lparse -lfile -lconio -lbare -lfpt -ldub

#LIB=lib/*.c lib/*.o
LIB=lib/*.o
SAFE_LIB=$(LIB:%.o=%-safe.o)
FAST_LIB=$(LIB:%.o=%-fast.o)


.PHONY: c0vm c0vmd clean
default: c0vm c0vmd

c0vm: c0vm.c c0vm_main.c
	$(CC_FAST) $(FAST_LIB) -o c0vm c0vm_main.c c0vm.c $(LINKERFLAGS)

c0vmd: c0vm.c c0vm_main.c
	$(CC_SAFE) $(SAFE_LIB) -o c0vmd c0vm_main.c c0vm.c $(LINKERFLAGS)

clean:
	rm -Rf c0vm c0vmd *.dSYM
