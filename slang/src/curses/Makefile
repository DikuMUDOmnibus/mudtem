COMPILE = $(CC) $(CFLAGS) -g -DSLANG -I..
LFLAGS = -L../$(ARCH)objs -lslang

CURSES_H = ../curses.h

all: $(CURSES_H) rain blue hanoi firework \
      bs battle gdc tclock worm view 
#knight newdemo testcurs xmas


$(CURSES_H):
	echo '#include <slcurses.h>' > $(CURSES_H)
 
view: view.c
	$(COMPILE) $@.c -o $@ $(LFLAGS)
bs: $(CURSES_H) bs.c
	$(COMPILE) $@.c -o $@ $(LFLAGS)
gdc: $(CURSES_H) gdc.c
	$(COMPILE) $@.c -o $@ $(LFLAGS)
battle: $(CURSES_H) battle.c
	$(COMPILE) $@.c -o $@ $(LFLAGS)
hanoi: $(CURSES_H) hanoi.c
	$(COMPILE) $@.c -o $@ $(LFLAGS)
blue: $(CURSES_H) blue.c
	$(COMPILE) $@.c -o $@ $(LFLAGS)
rain: $(CURSES_H) rain.c
	$(COMPILE) $@.c -o $@ $(LFLAGS)
firework: $(CURSES_H) firework.c
	$(COMPILE) $@.c -o $@ $(LFLAGS)
tclock: $(CURSES_H) tclock.c
	$(COMPILE) $@.c -o $@ $(LFLAGS) -lm
worm: worm.c
	$(COMPILE) $@.c -o $@ $(LFLAGS) -lm
knight: knight.c
	$(COMPILE) $@.c -o $@ $(LFLAGS) -lm
xmas: xmas.c
	$(COMPILE) $@.c -o $@ $(LFLAGS) -lm
newdemo: newdemo.c
	$(COMPILE) $@.c -o $@ $(LFLAGS) -lm
testcurs: testcurs.c
	$(COMPILE) $@.c -o $@ $(LFLAGS) -lm
lrtest: lrtest.c
	$(COMPILE) $@.c -o $@ $(LFLAGS) -lm
t: t.c
	$(COMPILE) $@.c -o $@ $(LFLAGS) -lm
key: key.c
	$(COMPILE) $@.c -o $@ $(LFLAGS) -lm
