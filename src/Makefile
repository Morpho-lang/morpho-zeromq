src = $(wildcard *.c) $(wildcard */*.c)
obj = $(src:.c=.dylib)

LDFLAGS  = -lm -lcblas -llapack -lcxsparse -lczmq -L/usr/local/lib
CFLAGS   =  -undefined dynamic_lookup -dynamiclib $(EXTCFLAGS) -O3 -I/usr/local/include/morpho

zeromq: 
	$(CC) -o $@.dylib $(src) $(LDFLAGS) $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) 

.PHONY: install
install: zeromq
	cp zeromq.dylib ../lib/zeromq.dylib
	make clean 
	