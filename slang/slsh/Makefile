# -*-sh-*-
# This Makefile is configured for Linux
#
SLANGINC = -I../src
SLANGLIB = -L../src/$(ARCH)objs
DL_LIB =  -ldl
LINK_FLAGS = -Wl,-export-dynamic # -static
MISC = -DHAVE_UNISTD_H
slsh: slsh.c
	$(CC) $(CFLAGS) -O2 slsh.c -o slsh $(MISC) $(SLANGINC) $(SLANGLIB) $(LDFLAGS) $(LINK_FLAGS) -lslang $(DL_LIB) -lm

clean:
	-/bin/rm -f *~ slsh scripts/*~

