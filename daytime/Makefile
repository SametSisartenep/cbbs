CC=cc
CFLAGS=-Wall -Wno-missing-braces -Wno-parentheses -Wno-switch -Wno-pointer-to-int-cast -fno-diagnostics-show-caret -fno-diagnostics-color -ggdb -c -O2
LDFLAGS=-pthread -lfmt -lutf
O=o

TARG=tcp13
OFILES=\
	main.$O\
	../util.$O\

HFILES=\
	../dat.h\
	../fns.h\
	../args.h\

.PHONY: all clean
all: $(TARG)

%.$O: %.c
	$(CC) $(CFLAGS) $<

$(OFILES): $(HFILES)

$(TARG): $(OFILES)
	$(CC) -o $@ $(OFILES:../%=%) $(LDFLAGS)

clean:
	rm $(OFILES:../%=%) $(TARG)
