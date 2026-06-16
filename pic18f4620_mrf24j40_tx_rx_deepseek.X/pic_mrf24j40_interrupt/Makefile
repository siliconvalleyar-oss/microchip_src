XC8 = xc8
CHIP = 18F4620
CFLAGS = --chip=$(CHIP) --opt=all
SOURCES = main.c mrf24j40.c spi.c usart.c oled.c

all: tx rx

tx: $(SOURCES)
	$(XC8) $(CFLAGS) -DMRF_MODE_TX $(SOURCES) -o firmware_tx.hex

rx: $(SOURCES)
	$(XC8) $(CFLAGS) -DMRF_MODE_RX $(SOURCES) -o firmware_rx.hex

clean:
	rm -f *.hex *.cof *.lst *.obj *.d *.p1 *.o

.PHONY: all clean tx rx
