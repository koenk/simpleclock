MCU = attiny87
PROGRAMMER = usbasp
CLOCKRATE = 1000000

PROGNAME = simpleclock

CC = avr-gcc
OBJCOPY = avr-objcopy
AR = avr-ar
AVRDUDE = avrdude

SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SOURCES))

GIT_VERSION := $(shell git describe --dirty="M" --tags --always 2>/dev/null || echo "nogit")

CFLAGS = -Os -Wall -Wextra -mmcu=$(MCU) -DF_CPU=$(CLOCKRATE)UL \
		 -DVERSION=\"$(GIT_VERSION)\"
LDFLAGS = -Os -mmcu=$(MCU)


.SUFFIXES:
.PRECIOUS: %.o %.elf
.PHONY: program install clean size

all: $(PROGNAME).elf size

program: $(PROGNAME).hex $(PROGNAME).eep
	avrdude -c $(PROGRAMMER) -p $(MCU) -u \
		-U flash:w:$(PROGNAME).hex:i \
        -U eeprom:w:$(PROGNAME).eep:i
install: program

size: ${PROGNAME}.elf
	@avr-size -C --mcu=${MCU} ${PROGNAME}.elf

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
%.elf: $(OBJS) $(LIBS)
	$(CC) $(LDFLAGS) -o $@ $^ $(SYSTEM_LIBS)
	@chmod a-x $@
%.eep: %.elf
	$(OBJCOPY) -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load \
        --no-change-warnings --change-section-lma .eeprom=0 $< $@
%.hex: %.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

clean:
	rm -f *.o *.elf *.eep *.hex
