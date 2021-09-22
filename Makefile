TARGET_DEVICE=atmega2560
CPUFREQ=1000000 
CC=/usr/bin/avr-gcc
OBJCOPY=/usr/bin/avr-objcopy
AR=/usr/bin/avr-ar
STRIP=/usr/bin/avr-strip

CFLAGS  = -g -Wall -Wextra -Werror -mmcu=$(TARGET_DEVICE) -O3 -DFCPU=$(CPUFREQ) -mcall-prologues

all: my

my: test/my.hex

MY_C25519 = obj/fe25519_add.o \
		obj/fe25519_red.o \
		obj/fe25519_sub.o \
		obj/bigint_mul256.o \
		obj/bigint_square256.o \
		#obj/bigint_subp.o \

test/my: test/my.c obj/my_c25519.a
	$(CC) $(CFLAGS) $^ -o $@

test/%.hex: test/%
	$(OBJCOPY) -O ihex -R .eeprom $^ $@

obj/my_c25519.a: $(MY_C25519)
	$(AR) -ar cr $@ $^

obj/%.o: %.[cS]
	mkdir -p obj/
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean

clean:
	-rm -r obj/*
	-rm -r test/my
