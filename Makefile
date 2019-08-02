CC=cc
CFLAGS=-Wall -Wno-missing-braces -Wno-parentheses -Wno-switch -ggdb -c -O2
LDFLAGS=-pthread -lfmt -lutf
O=o

TARG=csrv
OFILES=\
	main.$O\

HFILES=\
	args.h\

.PHONY: all clean
all: $(TARG)

%.$O: %.c
	$(CC) $(CFLAGS) $<

$(OFILES): $(HFILES)

$(TARG): $(OFILES)
	$(CC) -o $(TARG) $(OFILES) $(LDFLAGS)

clean:
	rm $(OFILES) $(TARG)