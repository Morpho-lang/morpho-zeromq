src = $(wildcard *.c) $(wildcard */*.c)
obj = $(src:.c=.dylib)

LDFLAGS  = -lm -lcblas -llapack -lcxsparse -lczmq -L/opt/homebrew/lib
CFLAGS   =  -undefined dynamic_lookup -dynamiclib $(EXTCFLAGS) -O3 -I/opt/homebrew/include

zeromq: 
	$(CC) -o $@.dylib $(src) $(LDFLAGS) $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) 

.PHONY: install
install: zeromq
	mkdir -p ../lib
	cp zeromq.dylib ../lib/zeromq.dylib
	make clean 
	