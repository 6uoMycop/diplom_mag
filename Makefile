TARGET_DEVICE=atmega2560
CPUFREQ=1000000 
CC=/usr/bin/avr-gcc
OBJCOPY=/usr/bin/avr-objcopy
AR=/usr/bin/avr-ar
STRIP=/usr/bin/avr-strip

CFLAGS  = -g -Wall -Wextra -Werror -mmcu=$(TARGET_DEVICE) -O3 -I../randombytes/ -I.. -I./include/ -DF_CPU=$(CPUFREQ) -mcall-prologues
MY_CFLAGS  = -g -Wall -Wextra -Werror -mmcu=$(TARGET_DEVICE) -O3 -mcall-prologues

all: test/my.hex \
	test/test.hex \
	test/speed.hex \
	test/stack.hex

my: test/my.hex

MY_C25519 = obj/fe25519_add.o \
		obj/fe25519_red.o \
		obj/fe25519_sub.o \
		obj/bigint_mul256.o \
		obj/bigint_square256.o \
		#obj/bigint_subp.o \

CURVE25519 = obj/fe25519.o \
		obj/fe25519_add.o \
		obj/fe25519_invert.o \
		obj/fe25519_mul121666.o \
		obj/fe25519_red.o \
		obj/fe25519_sub.o \
		obj/bigint_subp.o \
		obj/bigint_mul256.o \
		obj/bigint_square256.o \
		obj/curve25519.o 

TEST = test/print.c \
       test/avr.c \
       test/fail.c

SPEED = test/print.c \
        test/avr.c \
        test/fail.c \
        test/cpucycles.c



test/my: test/my.c obj/my_c25519.a
	$(CC) $(MY_CFLAGS) $^ -o $@

test/test: test/test.c obj/curve25519.a $(TEST)
	$(CC) $(CFLAGS) $^ -o $@

test/speed: test/speed.c obj/curve25519.a $(SPEED)
	$(CC) $(CFLAGS) $^ -o $@

test/stack: test/stack.c obj/curve25519.a $(TEST)
	$(CC) $(CFLAGS) $^ -o $@

test/%.hex: test/%
	$(OBJCOPY) -O ihex -R .eeprom $^ $@

obj/my_c25519.a: $(MY_C25519)
	$(AR) -ar cr $@ $^

obj/curve25519.a: $(CURVE25519)
	$(AR) -ar cr $@ $^

obj/%.o: %.[cS]
	mkdir -p obj/
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean

clean:
	-rm -r obj/*
	-rm -r test/my
	-rm -r test/test
	-rm -r test/speed
	-rm -r test/stack
