BIN := sha256_test

OBJS = main.o \
       sha256-avx-asm.o \
       sha256-avx2-asm.o \
       sha256-ssse3-asm.o \


CFLAGS := -std=gnu99 -O2 -g -m64

.PHONY: all
all: $(OBJS)
	gcc $(CFLAGS) -o $(BIN) $(OBJS)

%.o : %.c
	gcc -c $(CFLAGS) -o $@ $<

%.o : %.S
	gcc -c $(CFLAGS) -o $@ $<

%.o : %.s
	gcc -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -f $(BIN)
