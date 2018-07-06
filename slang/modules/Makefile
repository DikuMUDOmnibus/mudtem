# -*- sh -*-
CC_SHARED = $(CC) -shared -fPIC     # Linux
LINK_FLAGS = -Wl,-export-dynamic

SLANG_INC_DIR = ../src
SLANG_LIB_DIR = ../src/$(ARCH)objs

LIBS = -L$(SLANG_LIB_DIR) -lslang -ldl -lm
INCS = -I$(SLANG_INC_DIR)

MODULES = smg-module.so # newt-module.so

all: $(MODULES)

smg-module.so: smg-module.c
	$(CC_SHARED) $(CFLAGS) $(INCS) smg-module.c -o smg-module.so $(LIBS)
newt-module.so: newt-module.c
	$(CC_SHARED) $(CFLAGS) $(INCS) newt-module.c -o newt-module.so -lnewt $(LIBS)
clean:
	-/bin/rm -f *.so *~ *.o
